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

#ifndef ICONNECTIONREQUESTS_H
#define ICONNECTIONREQUESTS_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include "maemo/interfaces/itypes.h"
#include "maemo/interfaces/iabstract.h"
#include "maemo/interfaces/iconnection.h"
#include "maemo/dbusproxy.h"

namespace DBus
{
    namespace Interfaces
    {
        class IConnectionRequests : public DBus::IAbstract
        {
            Q_OBJECT

        public:
            static inline const char *staticInterfaceName()
            {
                return "org.freedesktop.Telepathy.Connection.Interface.Requests";
            }

            IConnectionRequests(
                const QString& busName,
                const QString& objectPath,
                QObject* parent = 0
            );
            IConnectionRequests(
                const QDBusConnection& connection,
                const QString& busName,
                const QString& objectPath,
                QObject* parent = 0
            );
            IConnectionRequests(DBus::DBusProxy *proxy);
            explicit IConnectionRequests(const DBus::Interfaces::IConnection& mainInterface);
            IConnectionRequests(const DBus::Interfaces::IConnection& mainInterface, QObject* parent);
            ~IConnectionRequests();

            Q_PROPERTY(DBus::Interfaces::ChannelDetailsList Channels READ Channels)
            inline DBus::Interfaces::ChannelDetailsList Channels() const
            {
                return qvariant_cast<DBus::Interfaces::ChannelDetailsList>(internalPropGet("Channels"));
            }

            Q_PROPERTY(DBus::Interfaces::RequestableChannelClassList RequestableChannelClasses READ RequestableChannelClasses)
            inline DBus::Interfaces::RequestableChannelClassList RequestableChannelClasses() const
            {
                return qvariant_cast<DBus::Interfaces::RequestableChannelClassList>(internalPropGet("RequestableChannelClasses"));
            }

        public Q_SLOTS:
            inline QDBusPendingReply<QDBusObjectPath, QVariantMap> CreateChannel(const QVariantMap& request)
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<QDBusObjectPath, QVariantMap>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                QList<QVariant> argumentList;
                argumentList << QVariant::fromValue(request);
                return asyncCallWithArgumentList(QLatin1String("CreateChannel"), argumentList);
            }
            inline QDBusPendingReply<bool, QDBusObjectPath, QVariantMap> EnsureChannel(const QVariantMap& request)
            {
                if (!invalidationReason().isEmpty()) {
                    return QDBusPendingReply<bool, QDBusObjectPath, QVariantMap>(QDBusMessage::createError(
                        invalidationReason(),
                        invalidationMessage()
                    ));
                }
                QList<QVariant> argumentList;
                argumentList << QVariant::fromValue(request);
                return asyncCallWithArgumentList(QLatin1String("EnsureChannel"), argumentList);
            }

        Q_SIGNALS:
            void NewChannels(const DBus::Interfaces::ChannelDetailsList& channels);
            void ChannelClosed(const QDBusObjectPath& removed);

        protected:
            virtual void invalidate(DBus::DBusProxy *, const QString &, const QString &);
        };
    }//Interfaces
}//DBus
Q_DECLARE_METATYPE(DBus::Interfaces::IConnectionRequests*)

#endif //ICONNECTIONREQUESTS_H