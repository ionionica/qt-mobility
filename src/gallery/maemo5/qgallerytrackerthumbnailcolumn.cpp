/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgallerytrackerthumbnailcolumn_p.h"

#include <QtCore/qcryptographichash.h>
#include <QtCore/qfile.h>
#include <QtCore/qfuturewatcher.h>
#include <QtCore/qtconcurrentmap.h>
#include <QtDBus/qdbuspendingcall.h>
#include <QtDBus/qdbuspendingreply.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>

QTM_BEGIN_NAMESPACE

class QGalleryTrackerThumbnailDBusWatcher : public QDBusPendingCallWatcher
{
public:
    QGalleryTrackerThumbnailDBusWatcher(
            const QDBusPendingCall &call,
            const QVector<uint> &imageIds,
            const QStringList &filePaths,
            const QStringList &mimeTypes)
        : QDBusPendingCallWatcher(call)
        , id(0)
        , imageIds(imageIds)
        , filePaths(filePaths)
        , mimeTypes(mimeTypes)
    {
    }

    uint id;
    QVector<uint> imageIds;
    QStringList filePaths;
    QStringList mimeTypes;
};

class QGalleryTrackerThumbnailLoadWatcher : public QFutureWatcher<QVariant>
{
public:
    QGalleryTrackerThumbnailLoadWatcher(
            const QVector<uint> imageIds, const QStringList &filePaths, const QStringList &mimeTypes)
        : imageIds(imageIds)
        , filePaths(filePaths)
        , mimeTypes(mimeTypes)
    {
    }

    QVector<uint> imageIds;
    QStringList filePaths;
    QStringList mimeTypes;
};


QGalleryTrackerThumbnailColumn::QGalleryTrackerThumbnailColumn(
        const QGalleryDBusInterfacePointer &thumbnailInterface,
        int key,
        int mimeTypeColumn,
        QVariant::Type type,
        const QString &flavor,
        QObject *parent)
    : QGalleryTrackerImageColumn(parent)
    , m_mimeTypeColumn(mimeTypeColumn)
    , m_type(type)
    , m_flavor(flavor)
    , m_thumbnailDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                 + QLatin1String("/.thumbnails/") + flavor + QLatin1Char('/'))
    , m_failDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)
                + QLatin1String("/.thumbnails/fail/"))
    , m_localDir(QLatin1String(".thumblocal/") + flavor + QLatin1Char('/'))
    , m_thumbnailInterface(thumbnailInterface)
    , m_keys(QList<int>() << key)
{
    connect(m_thumbnailInterface.data(), SIGNAL(Finished(uint)), this, SLOT(dbusFinished(uint)));
}

QGalleryTrackerThumbnailColumn::~QGalleryTrackerThumbnailColumn()
{
    qDeleteAll(m_dbusWatchers);

    typedef QList<QGalleryTrackerThumbnailLoadWatcher *>::iterator iterator;

    for (iterator it = m_loadWatchers.begin(), end = m_loadWatchers.end(); it != end; ++it)
        (*it)->cancel();

    for (iterator it = m_loadWatchers.begin(), end = m_loadWatchers.end(); it != end; ++it)
        (*it)->waitForFinished();

    qDeleteAll(m_loadWatchers);
}

QGalleryTrackerImageColumn *QGalleryTrackerThumbnailColumn::createImageColumn(
        QGalleryDBusInterfaceFactory *dbus,
        int key,
        const QString &profile,
        const QVector<int> &columns)
{
    return new QGalleryTrackerThumbnailColumn(
            dbus->thumbnailInterface(), key, columns.first(), QVariant::Image, profile);
}

QGalleryTrackerImageColumn *QGalleryTrackerThumbnailColumn::createPixmapColumn(
        QGalleryDBusInterfaceFactory *dbus,
        int key,
        const QString &profile,
        const QVector<int> &columns)
{
    return new QGalleryTrackerThumbnailColumn(
            dbus->thumbnailInterface(), key, columns.first(), QVariant::Pixmap, profile);
}

void QGalleryTrackerThumbnailColumn::insertImages(
        int index, int count, QVector<QVariant>::const_iterator row, int tableWidth)
{
    QStringList filePaths;
    QStringList mimeTypes;

    m_images.insert(index, count, QVariant());
    m_imageIds.insert(index, count, 0);

    for (int i = index; i < count; ++i, row += tableWidth) {
        const QString filePath = row->toString();
        const QString mimeType = (row + m_mimeTypeColumn)->toString();

        const uint imageId = qHash(filePath);

        m_imageIds.replace(i, imageId);

        filePaths.append(filePath);
        mimeTypes.append(mimeType);
    }

    QGalleryTrackerThumbnailLoadWatcher *watcher = new QGalleryTrackerThumbnailLoadWatcher(
            m_imageIds.mid(index, count), filePaths, mimeTypes);

    m_loadWatchers.append(watcher);

    connect(watcher, SIGNAL(finished()), this, SLOT(loadWatcherFinished()));
    connect(watcher, SIGNAL(resultsReadyAt(int,int)), this, SLOT(imagesReady(int,int)));

    watcher->setFuture(QtConcurrent::mapped(filePaths, LoadFunc(this)));
}

void QGalleryTrackerThumbnailColumn::removeImages(int index, int count)
{
    count = qMin(m_images.count() - index, count);

    if (count > 0) {
        m_images.remove(index, count);
        m_imageIds.remove(index, count);
    }
}

void QGalleryTrackerThumbnailColumn::loadWatcherFinished()
{
    QGalleryTrackerThumbnailLoadWatcher *loadWatcher
            = static_cast<QGalleryTrackerThumbnailLoadWatcher *>(sender());

    QStringList urls;
    QStringList mimeTypes;
    QStringList filePaths;
    QVector<uint> imageIds;

    for (int i = 0, count = loadWatcher->future().resultCount(); i < count; ++i) {
        if (loadWatcher->resultAt(i).isNull()) {
            urls.append(QUrl::fromLocalFile(loadWatcher->filePaths.at(i)).toString());
            mimeTypes.append(loadWatcher->mimeTypes.at(i));
            filePaths.append(loadWatcher->filePaths.at(i));
            imageIds.append(loadWatcher->imageIds.at(i));
        }
    }

    const uint cancelId = 0;

    if (!imageIds.isEmpty()) {
        QDBusPendingCall call = m_thumbnailInterface->asyncCall(
                QLatin1String("Queue"),
                urls,
                mimeTypes,
#ifndef Q_WS_MAEMO_5
                m_flavor,
                QLatin1String("default"),
#endif
                cancelId);

        QGalleryTrackerThumbnailDBusWatcher *dbusWatcher = new QGalleryTrackerThumbnailDBusWatcher(
                call, imageIds, filePaths, mimeTypes);

        m_dbusWatchers.append(dbusWatcher);

        if (dbusWatcher->isFinished()) {
            dbusWatcherFinished(dbusWatcher);
        } else {
            connect(dbusWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                    this, SLOT(dbusWatcherFinished(QDBusPendingCallWatcher*)));
        }
    }

    m_loadWatchers.removeAll(loadWatcher);

    loadWatcher->deleteLater();
}

void QGalleryTrackerThumbnailColumn::finalLoadWatcherFinished()
{
    QGalleryTrackerThumbnailLoadWatcher *loadWatcher
            = static_cast<QGalleryTrackerThumbnailLoadWatcher *>(sender());

    m_loadWatchers.removeAll(loadWatcher);

    loadWatcher->deleteLater();
}

void QGalleryTrackerThumbnailColumn::imagesReady(int begin, int end)
{
    QGalleryTrackerThumbnailLoadWatcher *watcher
            = static_cast<QGalleryTrackerThumbnailLoadWatcher *>(sender());

    for (int i = begin, index = 0, count = 0; i < end && index < m_imageIds.count(); ++i) {
        while (index < m_imageIds.count() && m_imageIds.at(index) != watcher->imageIds.at(i))
            ++index;

        while (i < end
               && index + count < m_imageIds.count()
               && m_imageIds.at(index + count) == watcher->imageIds.at(i)) {
            QVariant image = watcher->resultAt(i);

            if (!image.isNull()) {
                image.convert(m_type);

                m_images.replace(index + count, image);

                ++i;
                ++count;
            } else {
                if (count > 0)
                    emit imagesChanged(index + m_offset, count, m_keys);

                index += count;

                break;
            }
        }

        if (count > 0) {
            emit imagesChanged(index + m_offset, count, m_keys);

            index += count;
            count = 0;
        }
    }
}

void QGalleryTrackerThumbnailColumn::dbusWatcherFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError()) {
        qWarning("DBUS Error %s", qPrintable(watcher->error().message()));

        m_dbusWatchers.removeAll(static_cast<QGalleryTrackerThumbnailDBusWatcher *>(watcher));

        watcher->deleteLater();
    } else {
        QDBusPendingReply<uint> reply(*watcher);

        static_cast<QGalleryTrackerThumbnailDBusWatcher *>(watcher)->id = reply.value();
    }
}

void QGalleryTrackerThumbnailColumn::dbusError(
        uint id, const QStringList &uris, int error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(uris);

    qWarning("Thumbnail Error %d, %s", error, qPrintable(errorString));
}

void QGalleryTrackerThumbnailColumn::dbusFinished(uint id)
{
    for (QList<QGalleryTrackerThumbnailDBusWatcher *>::iterator it = m_dbusWatchers.begin();
            it != m_dbusWatchers.end();
            ++it) {
        if ((*it)->id == id) {

            QGalleryTrackerThumbnailDBusWatcher *dbusWatcher = *it;

            QGalleryTrackerThumbnailLoadWatcher *loadWatcher
                    = new QGalleryTrackerThumbnailLoadWatcher(
                            dbusWatcher->imageIds, dbusWatcher->filePaths, dbusWatcher->mimeTypes);

            m_loadWatchers.append(loadWatcher);

            connect(loadWatcher, SIGNAL(finished()), this, SLOT(finalLoadWatcherFinished()));
            connect(loadWatcher, SIGNAL(resultsReadyAt(int,int)), this, SLOT(imagesReady(int,int)));

            loadWatcher->setFuture(QtConcurrent::mapped(dbusWatcher->filePaths, LoadFunc(this)));

            dbusWatcher->deleteLater();

            m_dbusWatchers.erase(it);

            return;
        }
    }
}

QVariant QGalleryTrackerThumbnailColumn::loadThumbnail(const QString &filePath)
{
#ifdef Q_WS_MAEMO_5
    const QString hash = QCryptographicHash::hash(
            QUrl::fromLocalFile(filePath).toString().toUtf8(), QCryptographicHash::Md5).toHex();

    QString imagePath = m_thumbnailDir + hash + QLatin1String(".jpeg");
#else
    const QString hash = QCryptographicHash::hash(
            QUrl::fromLocalFile(filePath).toEncoded(), QCryptographicHash::Md5).toHex();

    QString imagePath = m_thumbnailDir + hash + QLatin1String(".png");
#endif

    if (!QFile::exists(imagePath)) {
        const int index = filePath.lastIndexOf(QLatin1Char('/'));

        if (index != -1 && index != filePath.count() - 1) {
            imagePath = filePath.mid(0, index + 1)
                    + m_localDir
#ifdef Q_WS_MAEMO_5
                    + QCryptographicHash::hash(
                            filePath.mid(index + 1).toUtf8(),
                            QCryptographicHash::Md5).toHex()
                    + QLatin1String(".jpeg");
#else
            + QCryptographicHash::hash(
                    QUrl::toPercentEncoding(filePath.mid(index + 1)),
                    QCryptographicHash::Md5).toHex()
                    + QLatin1String(".png");
#endif

            if (!QFile::exists(imagePath)) {
#ifdef Q_WS_MAEMO_5
                imagePath = m_failDir + hash + QLatin1String(".jpeg");
#else
                imagePath = m_failDir + hash + QLatin1String(".png");
#endif
                return QFile::exists(imagePath)
                        ? QImage()
                        : QVariant();
            }
        }
    }

    return QImage(imagePath);
}

#include "moc_qgallerytrackerthumbnailcolumn_p.cpp"

QTM_END_NAMESPACE