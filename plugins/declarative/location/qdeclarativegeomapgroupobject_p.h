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

#ifndef QDECLARATIVEGEOMAPGROUPOBJECT_H
#define QDECLARATIVEGEOMAPGROUPOBJECT_H

#include "qgeomapgroupobject.h"

#include "qdeclarativegeomapobject_p.h"
#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeListProperty>

QTM_BEGIN_NAMESPACE

class QGeoCoordinate;

class QDeclarativeGeoMapGroupObject : public QDeclarativeGeoMapObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeGeoMapObject> objects READ objects)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:
    QDeclarativeGeoMapGroupObject(QDeclarativeItem *parent = 0);
    ~QDeclarativeGeoMapGroupObject();

    virtual void componentComplete();

    virtual void setMap(QDeclarativeGraphicsGeoMap *map);

    QDeclarativeListProperty<QDeclarativeGeoMapObject> objects();

    virtual void doubleClickEvent(QDeclarativeGeoMapMouseEvent *event);
    virtual void pressEvent(QDeclarativeGeoMapMouseEvent *event);
    virtual void releaseEvent(QDeclarativeGeoMapMouseEvent *event);
    virtual void enterEvent();
    virtual void exitEvent();
    virtual void moveEvent(QDeclarativeGeoMapMouseEvent *event);

    void setVisible(bool visible);
    bool isVisible() const;

Q_SIGNALS:
    void visibleChanged(bool visible);

private:
    static void child_append(QDeclarativeListProperty<QDeclarativeGeoMapObject> *prop, QDeclarativeGeoMapObject *mapObject);
    static int child_count(QDeclarativeListProperty<QDeclarativeGeoMapObject> *prop);
    static QDeclarativeGeoMapObject* child_at(QDeclarativeListProperty<QDeclarativeGeoMapObject> *prop, int index);
    static void child_clear(QDeclarativeListProperty<QDeclarativeGeoMapObject> *prop);

    QGeoMapGroupObject* group_;
    QList<QDeclarativeGeoMapObject*> objects_;
    bool visible_;
};

QTM_END_NAMESPACE

QML_DECLARE_TYPE(QTM_PREPEND_NAMESPACE(QDeclarativeGeoMapGroupObject));

#endif
