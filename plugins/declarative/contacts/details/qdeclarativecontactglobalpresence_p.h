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


#ifndef QDECLARATIVECONTACTGLOBALPRESENCE_H
#define QDECLARATIVECONTACTGLOBALPRESENCE_H

#include "qdeclarativecontactdetail_p.h"
#include "qcontactglobalpresence.h"
#include "qdeclarativecontactpresence_p.h"

class QDeclarativeContactGlobalPresence : public QDeclarativeContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY fieldsChanged)
    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY fieldsChanged)
    Q_PROPERTY(QDeclarativeContactPresence::PresenceStateType state READ presenceState WRITE setPresenceState NOTIFY fieldsChanged)
    Q_PROPERTY(QString stateText READ presenceStateText WRITE setPresenceStateText NOTIFY fieldsChanged)
    Q_PROPERTY(QUrl imageUrl READ presenceStateImageUrl WRITE setPresenceStateImageUrl NOTIFY fieldsChanged)
    Q_PROPERTY(QString customMessage READ customMessage WRITE setCustomMessage NOTIFY fieldsChanged)
    Q_CLASSINFO("DefaultProperty", "state")
    Q_ENUMS(FieldType)
public:
    enum FieldType {
        Timestamp = 0,
        Nickname,
        State,
        StateText,
        ImageUrl,
        CustomMessage
    };

    QDeclarativeContactGlobalPresence(QObject* parent = 0)
        :QDeclarativeContactDetail(parent)
    {
        setDetail(QContactGlobalPresence());
        connect(this, SIGNAL(fieldsChanged()), SIGNAL(valueChanged()));
    }
    ContactDetailType detailType() const
    {
        return QDeclarativeContactDetail::GlobalPresence;
    }
    static QString fieldNameFromFieldType(int fieldType)
    {
        switch (fieldType) {
        case Timestamp:
            return QContactGlobalPresence::FieldTimestamp;
        case Nickname:
            return QContactGlobalPresence::FieldNickname;
        case State:
            return QContactGlobalPresence::FieldPresenceState;
        case StateText:
            return QContactGlobalPresence::FieldPresenceStateText;
        case ImageUrl:
            return QContactGlobalPresence::FieldPresenceStateImageUrl;
        case CustomMessage:
            return QContactGlobalPresence::FieldCustomMessage;
        default:
            break;
        }
        qmlInfo(0) << tr("Unknown field type.");
        return QString();
    }
    void setTimestamp(const QDateTime& v)
    {
        if (!readOnly() && v != timestamp()) {
            detail().setValue(QContactGlobalPresence::FieldTimestamp, v);
            emit fieldsChanged();
        }
    }
    QDateTime timestamp() const {return detail().value<QDateTime>(QContactGlobalPresence::FieldTimestamp);}
    void setNickname(const QString& v)
    {
        if (!readOnly() && v != nickname()) {
            detail().setValue(QContactGlobalPresence::FieldNickname, v);
            emit fieldsChanged();
        }
    }
    QString nickname() const {return detail().value(QContactGlobalPresence::FieldNickname);}
    void setPresenceState(QDeclarativeContactPresence::PresenceStateType v)
    {
        if (!readOnly() && v != presenceState()) {
            detail().setValue(QContactGlobalPresence::FieldPresenceState, static_cast<int>(v));
            emit fieldsChanged();
        }
    }

    QDeclarativeContactPresence::PresenceStateType presenceState() const
    {
        return static_cast<QDeclarativeContactPresence::PresenceStateType>(detail().value<int>(QContactGlobalPresence::FieldPresenceState));

    }
    void setPresenceStateText(const QString& v)
    {
        if (!readOnly() && v != presenceStateText()) {
            detail().setValue(QContactGlobalPresence::FieldPresenceStateText, v);
            emit fieldsChanged();
        }
    }
    QString presenceStateText() const {return detail().value(QContactGlobalPresence::FieldPresenceStateText);}
    void setPresenceStateImageUrl(const QUrl& v)
    {
        if (!readOnly() && v != presenceStateImageUrl())  {
            detail().setValue(QContactGlobalPresence::FieldPresenceStateImageUrl, v);
            emit fieldsChanged();
        }
    }
    QUrl presenceStateImageUrl() const {return detail().value<QUrl>(QContactGlobalPresence::FieldPresenceStateImageUrl);}
    void setCustomMessage(const QString& v)
    {
        if (!readOnly() && v != customMessage()) {
            detail().setValue(QContactGlobalPresence::FieldCustomMessage, v);
            emit fieldsChanged();
        }
    }
    QString customMessage() const {return detail().value(QContactGlobalPresence::FieldCustomMessage);}
signals:
    void fieldsChanged();
};
QML_DECLARE_TYPE(QDeclarativeContactGlobalPresence)

#endif

