/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>


#include "s60serviceplugin.h"
#include "s60radiotunerservice.h"
//#include "s60cameraservice.h" //Camera impl is on hold
#ifdef HAS_MEDIA_PLAYER
#include "s60mediaplayerservice.h"
#endif
#include "s60audiocaptureservice.h"

QStringList S60ServicePlugin::keys() const
{
    QStringList list;
    list << QLatin1String(Q_MEDIASERVICE_RADIO);
  //  list << QLatin1String(Q_MEDIASERVICE_CAMERA); //Camera impl is on hold
#ifdef HAS_MEDIA_PLAYER  
    list << QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER);
#endif    
    list << QLatin1String(Q_MEDIASERVICE_AUDIOSOURCE);
    return list;
}

QMediaService* S60ServicePlugin::create(QString const& key)
{
    if (key == QLatin1String(Q_MEDIASERVICE_RADIO))
        return new S60RadioTunerService;
 /*   else if (key == QLatin1String(Q_MEDIASERVICE_CAMERA))
        return new S60CameraService;*/ //Camera impl is on hold
#ifdef HAS_MEDIA_PLAYER
    else if (key == QLatin1String(Q_MEDIASERVICE_MEDIAPLAYER))
        return new S60MediaPlayerService;
#endif
    else if (key == QLatin1String(Q_MEDIASERVICE_AUDIOSOURCE))
        return new S60AudioCaptureService;

    qDebug() << "unsupported key:" << key;
    return 0;
}
void S60ServicePlugin::release(QMediaService *service)
{
    delete service;
}
/* Camera impl is on hold
QList<QByteArray> S60ServicePlugin::devices(const QByteArray &service) const
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        if (m_cameraDevices.isEmpty())
            updateDevices();

        return m_cameraDevices;
    }

    return QList<QByteArray>();
}

QString S60ServicePlugin::deviceDescription(const QByteArray &service, const QByteArray &device)
{
    if (service == Q_MEDIASERVICE_CAMERA) {
        if (m_cameraDevices.isEmpty())
            updateDevices();

        for (int i=0; i<m_cameraDevices.count(); i++)
            if (m_cameraDevices[i] == device)
                return m_cameraDescriptions[i];
    }

    return QString();
}

void S60ServicePlugin::updateDevices() const
{

    m_cameraDevices.clear();
    m_cameraDescriptions.clear();
    for (int i=0; i < S60CameraService::deviceCount(); i ++) {
        QString deviceName = QString().number(i);
        m_cameraDevices.append(deviceName.toUtf8());
        QString deviceDesc = S60CameraService::deviceDescription(i);
        m_cameraDescriptions.append(deviceDesc);
    }

}
*/
Q_EXPORT_PLUGIN2(QtMobilityMultimediaEngine, S60ServicePlugin);