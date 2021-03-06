/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
***************************************************************************/

#ifndef QDECLARATIVELANDMARK_P_H
#define QDECLARATIVELANDMARK_P_H

#include <QtCore>
#include <QAbstractListModel>
#include <qlandmark.h>
#include <QtDeclarative/qdeclarative.h>
#include "qdeclarativelandmarkcategory_p.h"
#include "qdeclarativecoordinate_p.h"
#include "qdeclarativegeoplace_p.h"

// Define this to get qDebug messages
// #define QDECLARATIVE_LANDMARK_DEBUG

#ifdef QDECLARATIVE_LANDMARK_DEBUG
#include <QDebug>
#endif

QTM_BEGIN_NAMESPACE

class QDeclarativeLandmark : public QDeclarativeGeoPlace
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber WRITE setPhoneNumber NOTIFY phoneNumberChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QUrl iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit QDeclarativeLandmark(QObject* parent = 0);
    QDeclarativeLandmark(const QLandmark& landmark, QObject* parent = 0);
    void setLandmark(const QLandmark& landmark);

    QString name();
    void setName(const QString& name);
    QString phoneNumber();
    void setPhoneNumber(const QString& phoneNumber);
    QString description();
    void setDescription(const QString& description);
    double radius();
    void setRadius(const double& radius);
    QUrl iconSource();
    void setIconSource(const QUrl& iconSource);
    QUrl url();
    void setUrl(const QUrl& url);
    QDeclarativeCoordinate* coordinate();
    void setCoordinate(QDeclarativeCoordinate* coordinate);

    QLandmark landmark();

signals:
    void nameChanged();
    void phoneNumberChanged();
    void descriptionChanged();
    void radiusChanged();
    void iconSourceChanged();
    void urlChanged();
    void coordinateChanged();

private:
    friend class QDeclarativeLandmarkModel;
    friend class QDeclarativeLandmarkCategoryModel;
    QList<QLandmarkCategoryId> categoryIds () const;
    QDeclarativeCoordinate m_coordinate;
    QLandmark m_landmark;
};

QTM_END_NAMESPACE
QML_DECLARE_TYPE(QTM_PREPEND_NAMESPACE(QDeclarativeLandmark));

#endif // QDECLARATIVELANDMARK_P_H
