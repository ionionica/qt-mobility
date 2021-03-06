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
****************************************************************************/

#include "qmessageservice.h"

#include <QObject>

QTM_BEGIN_NAMESPACE

class QMessageServicePrivate : public QObject
{
    Q_OBJECT

public:
    enum EnginesToCall
    {
        EnginesToCallModest    = 0x1,
        EnginesToCallTelepathy = 0x2,
        // Extensible
        EnginesToCallAll = 0xFFFFFFFF,
    };

    QMessageServicePrivate(QMessageService* parent);
    ~QMessageServicePrivate();

    static QMessageServicePrivate* implementation(const QMessageService &service);

    bool queryMessages(QMessageService &messageService, const QMessageFilter &filter,
                       const QMessageSortOrder &sortOrder, uint limit, uint offset,
                       EnginesToCall enginesToCall = EnginesToCallAll);
    bool queryMessages(QMessageService &messageService, const QMessageFilter &filter,
                       const QString &body, QMessageDataComparator::MatchFlags matchFlags,
                       const QMessageSortOrder &sortOrder, uint limit, uint offset,
                       EnginesToCall enginesToCall = EnginesToCallAll);
    bool countMessages(QMessageService &messageService, const QMessageFilter &filter,
                       EnginesToCall enginesToCall = EnginesToCallAll);

    void setFinished(bool successful);
    void messagesCounted(int count);
    void progressChanged(uint value, uint total);

public slots:
    void finishedSlot(bool successful = true);
    void messagesFoundSlot();
    void messagesCountedSlot();
    void messagesFound(const QMessageIdList &ids, bool isFiltered, bool isSorted);
    void stateChanged(QMessageService::State state);
public:
    QMessageService* q_ptr;
    QMessageService::State _state;
    QMessageManager::Error _error;
    bool _active;
    int _actionId;
    int _pendingRequestCount;

    QMessageIdList _ids;
    int _count;
    bool _sorted;
    bool _filtered;

    QMessageFilter _filter;
    QMessageSortOrder _sortOrder;
    int _limit;
    int _offset;
};

QTM_END_NAMESPACE
