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

#ifndef ICHANNELTYPETEXT_H
#define ICHANNELTYPETEXT_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include "maemo/interfaces/itypes.h"
#include "maemo/interfaces/iabstract.h"
#include "maemo/interfaces/ichannel.h"
#include "maemo/dbusproxy.h"

namespace DBus
{
    namespace Interfaces
    {
        class IChannelTypeText : public DBus::IAbstract
        {
            Q_OBJECT

        public:
            static inline const char *staticInterfaceName()
            {
                return "org.freedesktop.Telepathy.Channel.Type.Text";
            }

            IChannelTypeText(
                const QString& busName,
                const QString& objectPath,
                QObject* parent = 0
            );
            IChannelTypeText(
                const QDBusConnection& connection,
                const QString& busName,
                const QString& objectPath,
                QObject* parent = 0
            );
            IChannelTypeText(DBus::DBusProxy *proxy);
            explicit IChannelTypeText(const DBus::Interfaces::IChannel& mainInterface);
            IChannelTypeText(const DBus::Interfaces::IChannel& mainInterface, QObject* parent);

        public Q_SLOTS:
            inline QDBusPendingReply<> AcknowledgePendingMessages(const DBus::Interfaces::UIntList& IDs)
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                QList<QVariant> argumentList;
                argumentList << QVariant::fromValue(IDs);
                return asyncCallWithArgumentList(QLatin1String("AcknowledgePendingMessages"), argumentList);
            }
            inline QDBusPendingReply<DBus::Interfaces::UIntList> GetMessageTypes()
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<DBus::Interfaces::UIntList>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                return asyncCall(QLatin1String("GetMessageTypes"));
            }
            inline QDBusPendingReply<DBus::Interfaces::PendingTextMessageList> ListPendingMessages(bool clear)
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<DBus::Interfaces::PendingTextMessageList>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                QList<QVariant> argumentList;
                argumentList << QVariant::fromValue(clear);
                return asyncCallWithArgumentList(QLatin1String("ListPendingMessages"), argumentList);
            }
            inline QDBusPendingReply<> Send(uint type, const QString& text)
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                QList<QVariant> argumentList;
                argumentList << QVariant::fromValue(type) << QVariant::fromValue(text);
                return asyncCallWithArgumentList(QLatin1String("Send"), argumentList);
            }

        Q_SIGNALS:
            void LostMessage();
            void Received(uint ID, uint timestamp, uint sender, uint type, uint flags, const QString& text);
            void SendError(uint error, uint timestamp, uint type, const QString& text);
            void Sent(uint timestamp, uint type, const QString& text);

        protected:
            virtual void invalidate(DBus::DBusProxy *, const QString &, const QString &);
        };
    }
}
Q_DECLARE_METATYPE(DBus::Interfaces::IChannelTypeText*)

#endif //ICHANNELTYPETEXT_H