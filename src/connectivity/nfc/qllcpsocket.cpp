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


#include "qllcpsocket.h"

#if defined(QT_SIMULATOR)
#include "qllcpsocket_simulator_p.h"
#elif defined(Q_OS_SYMBIAN)
#include "qllcpsocket_symbian_p.h"
#else
#include "qllcpsocket_p.h"
#endif

QTM_BEGIN_NAMESPACE

/*!
    \class QLlcpSocket
    \brief The QLlcpSocket class provides an NFC LLCP socket.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity

    NFC LLCP protocol is a peer-to-peer communication protocol between two NFC complient devices.
*/

/*!
    \enum QLlcpSocket::Error

    This enum describes the errors that can occur. The most recent error can be retrieved through a
    call to error().

    \value UnknownSocketError   An unidentified error has occurred.
*/

/*!
    \enum QLlcpSocket::State

    This enum describes the different state in which a socket can be.

    \value UnconnectedState The socket is not connected.
    \value ConnectingState  The socket has started establishing a connection.
    \value ConnectedState   A connection is established.
    \value ClosingState     The socket is about to close.
*/

/*!
    \fn QLlcpSocket::connected()

    This signal is emitted after connectToService() has been called and a connection has been
    successfully established.

    \sa connectToService(), disconnected()
*/

/*!
    \fn QLlcpSocket::disconnected()

    This signal is emitted when the socket has been disconnected.

    \sa disconnectFromService(),
*/

/*!
    \fn QLlcpSocket::error(QLlcpSocket::Error socketError)

    This signal is emitted when an error occurs. The \a socketError parameter describes the error.
*/

/*!
    \fn QLlcpSocket::stateChanged(QLlcpSocket::State socketState)

    This signal is emitted when the state of the socket changes. The \a socketState parameter
    describes the new state.
*/

/*!
    Construct a new unconnected LLCP socket with \a parent.
*/
QLlcpSocket::QLlcpSocket(QObject *parent)
:   QIODevice(parent), d_ptr(new QLlcpSocketPrivate)
{
}

/*!
    Destroys the LLCP socket.
*/
QLlcpSocket::~QLlcpSocket()
{
    delete d_ptr;
}

/*!
    Connects to the service identified by the URI \a serviceUri on \a target.
*/
void QLlcpSocket::connectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    Q_D(QLlcpSocket);

    d->connectToService(target, serviceUri);
}

/*!
    Disconnects the socket.
*/
void QLlcpSocket::disconnectFromService()
{
    Q_D(QLlcpSocket);

    d->disconnectFromService();
}

/*!
    Binds the LLCP socket to local \a port. Returns true on success; otherwise returns false.
*/
bool QLlcpSocket::bind(quint8 port)
{
    Q_D(QLlcpSocket);

    return d->bind(port);
}

/*!
    Returns true if at least one datagram (service data units) is waiting to be read; otherwise
    returns false.

    \sa pendingDatagramSize(), readDatagram()
*/
bool QLlcpSocket::hasPendingDatagrams() const
{
    Q_D(const QLlcpSocket);

    return d->hasPendingDatagrams();
}

/*!
    Returns the size of the first pending datagram (service data unit). If there is no datagram
    available, this function returns -1.

    \sa hasPendingDatagrams(), readDatagram()
*/
qint64 QLlcpSocket::pendingDatagramSize() const
{
    Q_D(const QLlcpSocket);

    return d->pendingDatagramSize();
}

/*!
    Sends the datagram at \a data of size \a size to the service that this socket is connected to.
    Returns the number of bytes sent on success; otherwise return -1;
*/
qint64 QLlcpSocket::writeDatagram(const char *data, qint64 size)
{
    Q_D(QLlcpSocket);

    return d->writeDatagram(data, size);
}

/*!
    \overload

    Sends the datagram \a datagram to the service that this socket is connected to.
*/
qint64 QLlcpSocket::writeDatagram(const QByteArray &datagram)
{
    Q_D(QLlcpSocket);

    return d->writeDatagram(datagram);
}

/*!
    Receives a datagram no larger than \a maxSize bytes and stores it in \a data. The sender's
    details are stored in \a target and \a port (unless the pointers are 0).

    Returns the size of the datagram on success; otherwise returns -1.

    If maxSize is too small, the rest of the datagram will be lost. To avoid loss of data, call
    pendingDatagramSize() to determine the size of the pending datagram before attempting to read
    it. If maxSize is 0, the datagram will be discarded.

    \sa writeDatagram(), hasPendingDatagrams(), pendingDatagramSize()
*/
qint64 QLlcpSocket::readDatagram(char *data, qint64 maxSize, QNearFieldTarget **target,
                                 quint8 *port)
{
    Q_D(QLlcpSocket);

    return d->readDatagram(data, maxSize, target, port);
}

/*!
    Sends the datagram at \a data of size \a size to the service identified by the URI
    \a port on \a target. Returns the number of bytes sent on success; otherwise returns -1.

    \sa readDatagram()
*/
qint64 QLlcpSocket::writeDatagram(const char *data, qint64 size, QNearFieldTarget *target,
                                  quint8 port)
{
    Q_D(QLlcpSocket);

    return d->writeDatagram(data, size, target, port);
}

/*!
    \overload

    Sends the datagram \a datagram to the service identified by the URI \a port on \a target.
*/
qint64 QLlcpSocket::writeDatagram(const QByteArray &datagram, QNearFieldTarget *target,
                                  quint8 port)
{
    Q_D(QLlcpSocket);

    return d->writeDatagram(datagram, target, port);
}

/*!
    Returns the type of error that last occurred.
*/
QLlcpSocket::Error QLlcpSocket::error() const
{
    Q_D(const QLlcpSocket);

    return d->error();
}

/*!
    \internal
*/
qint64 QLlcpSocket::readData(char *data, qint64 maxlen)
{
    Q_D(QLlcpSocket);

    return d->readData(data, maxlen);
}

/*!
    \internal
*/
qint64 QLlcpSocket::writeData(const char *data, qint64 len)
{
    Q_D(QLlcpSocket);

    return d->writeData(data, len);
}

#include <moc_qllcpsocket.cpp>

QTM_END_NAMESPACE