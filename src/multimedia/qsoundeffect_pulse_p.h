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

#ifndef QSOUNDEFFECT_PULSE_H
#define QSOUNDEFFECT_PULSE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QtCore/qobject.h>

#include <QTime>

#include <qmediaplayer.h>

#include "qsoundeffect_p.h"

#include <pulse/pulseaudio.h>

QTM_USE_NAMESPACE

class QNetworkReply;
class QNetworkAccessManager;
class WaveDecoder;

class QSoundEffectPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QSoundEffectPrivate(QObject* parent);
    ~QSoundEffectPrivate();

    qint64 duration() const;
    int volume() const;
    bool isMuted() const;
    QMediaContent media() const;
    QMediaPlayer::State state() const;
    QMediaPlayer::MediaStatus mediaStatus() const;

public Q_SLOTS:
    void play();
    void stop();
    void setVolume(int volume);
    void setMuted(bool muted);
    void setMedia(const QMediaContent &media);

Q_SIGNALS:
    void mediaChanged(const QMediaContent &media);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void stateChanged(QMediaPlayer::State newState);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void error(QMediaPlayer::Error error);

private slots:
    void decoderReady();
    void decoderError();
    void checkPlayTime();

private:
    void loadSample();
    void unloadSample();

    void timerEvent(QTimerEvent *event);

    static void stream_write_callback(pa_stream *s, size_t length, void *userdata);
    static void stream_state_callback(pa_stream *s, void *userdata);
    static void play_callback(pa_context *c, int success, void *userdata);

    bool    m_muted;
    bool    m_playQueued;
    int     m_volume;
    int     m_duration;
    int     m_dataUploaded;
    QTime  m_playbackTime;
    QMediaPlayer::State m_state;
    QMediaPlayer::MediaStatus m_status;
    QByteArray m_name;
    QMediaContent   m_media;
    QNetworkReply *m_reply;
    WaveDecoder *m_waveDecoder;
    QIODevice *m_stream;
    QNetworkAccessManager *m_networkAccessManager;
};

#endif // QSOUNDEFFECT_PULSE_H