/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVECONTACTRELATIONSHIPMODEL_P_H
#define QDECLARATIVECONTACTRELATIONSHIPMODEL_P_H

#include <qdeclarative.h>
#include <QAbstractListModel>

#include "qdeclarativecontactrelationship_p.h"

QTM_USE_NAMESPACE;
class QDeclarativeContactRelationshipModelPrivate;
class QDeclarativeContactRelationshipModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString manager READ manager WRITE setManager NOTIFY managerChanged)
    Q_PROPERTY(QContactLocalId participantId READ participantId WRITE setParticipantId NOTIFY participantIdChanged)
    Q_PROPERTY(QVariant relationshipType READ relationshipType WRITE setRelationshipType NOTIFY relationshipTypeChanged)
    Q_PROPERTY(QDeclarativeContactRelationship::RelationshipRole role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QString error READ error)

public:
    QDeclarativeContactRelationshipModel(QObject *parent = 0);
    ~QDeclarativeContactRelationshipModel();
    enum {
        RelationshipRole = Qt::UserRole + 500

    };

    QString manager() const;
    void setManager(const QString& manager);
    QString error() const;
    QContactLocalId participantId() const;
    void setParticipantId(const QContactLocalId& id);

    QVariant relationshipType() const;
    void setRelationshipType(const QVariant& type);

    QDeclarativeContactRelationship::RelationshipRole role() const;
    void setRole(QDeclarativeContactRelationship::RelationshipRole role);


    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void removeRelationship(QDeclarativeContactRelationship* dcr);
    Q_INVOKABLE void addRelationship(QDeclarativeContactRelationship* dcr);
signals:
    void managerChanged();
    void participantIdChanged();
    void relationshipTypeChanged();
    void roleChanged();
    void relationshipsChanged();

private slots:
    void fetchAgain();
    void requestUpdated();

    void relationshipsSaved();

    void relationshipsRemoved();

private:
    QDeclarativeContactRelationshipModelPrivate* d;
};

QML_DECLARE_TYPE(QDeclarativeContactRelationshipModel)
#endif // QDECLARATIVECONTACTRELATIONSHIPMODEL_P_H