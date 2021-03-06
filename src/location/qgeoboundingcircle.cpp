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

#include "qgeoboundingcircle.h"
#include "qgeoboundingcircle_p.h"

#include "qgeocoordinate.h"
#include "qnumeric.h"

#include <QDebug>

QTM_BEGIN_NAMESPACE

/*!
    \class QGeoBoundingCircle
    \brief The QGeoBoundingCircle class defines a circular geographic area.

    \inmodule QtLocation
    \since 1.1

    \ingroup maps

    The circle is defined in terms of a QGeoCoordinate which specifies the
    center of the circle and a qreal which specifies the radius of the circle
    in metres.

    The circle is considered invalid if the center coordinate is invalid
    or if the radius is less than zero.
*/

/*!
    Constructs a new, invalid bounding circle.
*/
QGeoBoundingCircle::QGeoBoundingCircle()
        : d_ptr(new QGeoBoundingCirclePrivate()) {}

/*!
    Constructs a new bounding circle centered at \a center and with a radius of \a
    radius metres.
    \since 1.1
*/
QGeoBoundingCircle::QGeoBoundingCircle(const QGeoCoordinate &center, qreal radius)
        : d_ptr(new QGeoBoundingCirclePrivate(center, radius)) {}

/*!
    Constructs a new bounding circle from the contents of \a other.
    \since 1.1
*/
QGeoBoundingCircle::QGeoBoundingCircle(const QGeoBoundingCircle &other)
        : QGeoBoundingArea(other),
          d_ptr(other.d_ptr) {}

/*!
    Destroys this bounding circle.
*/
QGeoBoundingCircle::~QGeoBoundingCircle() {}

/*!
    Assigns \a other to this bounding circle and returns a reference to this
    bounding circle.
    \since 1.1
*/
QGeoBoundingCircle& QGeoBoundingCircle::operator = (const QGeoBoundingCircle & other)
{
    QGeoBoundingArea::operator=(other);
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns whether this bounding circle is equal to \a other.
    \since 1.1
*/
bool QGeoBoundingCircle::operator == (const QGeoBoundingCircle &other) const
{
    return ((*(d_ptr.constData()) == *(other.d_ptr.constData())));
}

/*!
    Returns whether this bounding circle is not equal to \a other.
    \since 1.1
*/
bool QGeoBoundingCircle::operator != (const QGeoBoundingCircle &other) const
{
    return !(this->operator==(other));
}

/*!
    Returns QGeoBoundingArea::CircleType to identify this as a
    QGeoBoundingCircle instance.

    This function is provided to help find the specific type of
    aQGeoBoundingArea instance.
    \since 1.1
*/
QGeoBoundingArea::AreaType QGeoBoundingCircle::type() const
{
    return QGeoBoundingArea::CircleType;
}

/*!
    Returns whether this bounding circle is valid.

    A valid bounding circle has a valid center coordinate and a radius
    greater than or equal to zero.
    \since 1.1
*/
bool QGeoBoundingCircle::isValid() const
{
    return (d_ptr->center.isValid()
            && !qIsNaN(d_ptr->radius)
            && d_ptr->radius >= -1e-7);
}

/*!
    Returns whether this bounding circle has a geometrical area of zero.

    Returns true if this bounding circle is invalid.
    \since 1.1
*/
bool QGeoBoundingCircle::isEmpty() const
{
    return (!isValid() || (d_ptr->radius <= 1e-7));
}

/*!
    Sets the center coordinate of this bounding circle to \a center.
    \since 1.1
*/
void QGeoBoundingCircle::setCenter(const QGeoCoordinate &center)
{
    d_ptr->center = center;
}

/*!
    Returns the center coordinate of this bounding circle.
    \since 1.1
*/
QGeoCoordinate QGeoBoundingCircle::center() const
{
    return d_ptr->center;
}

/*!
    Sets the radius in meters of this bounding circle to \a radius.
    \since 1.1
*/
void QGeoBoundingCircle::setRadius(qreal radius)
{
    d_ptr->radius = radius;
}

/*!
    Returns the radius in meters of this bounding circle.
    \since 1.1
*/
qreal QGeoBoundingCircle::radius() const
{
    return d_ptr->radius;
}

/*!
    Returns whether the coordinate \a coordinate is contained within this
    bounding circle.
    \since 1.1
*/
bool QGeoBoundingCircle::contains(const QGeoCoordinate &coordinate) const
{

    if (!isValid() || !coordinate.isValid())
        return false;

    if (d_ptr->center.distanceTo(coordinate) <= d_ptr->radius)
        return true;

    return false;
}

/*!
    Translates this bounding circle by \a degreesLatitude northwards and \a
    degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.
    \since 1.1
*/
void QGeoBoundingCircle::translate(double degreesLatitude, double degreesLongitude)
{
    // TODO handle dlat, dlon larger than 360 degrees

    double lat = d_ptr->center.latitude();
    double lon = d_ptr->center.longitude();

    lat += degreesLatitude;
    lon += degreesLongitude;

    if (lon < -180.0)
        lon += 360.0;
    if (lon > 180.0)
        lon -= 360.0;

    if (lat > 90.0) {
        lat = 180.0 - lat;
        if (lon < 0.0)
            lon = 180.0;
        else
            lon -= 180;
    }

    if (lat < -90.0) {
        lat = 180.0 + lat;
        if (lon < 0.0)
            lon = 180.0;
        else
            lon -= 180;
    }

    d_ptr->center = QGeoCoordinate(lat, lon);
}

/*!
    Returns a copy of this bounding circle translated by \a degreesLatitude northwards and \a
    degreesLongitude eastwards.

    Negative values of \a degreesLatitude and \a degreesLongitude correspond to
    southward and westward translation respectively.

    \since 1.1
    \sa translate()
*/
QGeoBoundingCircle QGeoBoundingCircle::translated(double degreesLatitude, double degreesLongitude) const
{
    QGeoBoundingCircle result(*this);
    result.translate(degreesLatitude, degreesLongitude);
    return result;
}

/*******************************************************************************
*******************************************************************************/

QGeoBoundingCirclePrivate::QGeoBoundingCirclePrivate()
    : QSharedData(),
    radius(-1.0) {}

QGeoBoundingCirclePrivate::QGeoBoundingCirclePrivate(const QGeoCoordinate &center, qreal radius)
        : QSharedData(),
        center(center),
        radius(radius) {}

QGeoBoundingCirclePrivate::QGeoBoundingCirclePrivate(const QGeoBoundingCirclePrivate &other)
        : QSharedData(),
        center(other.center),
        radius(other.radius) {}

QGeoBoundingCirclePrivate::~QGeoBoundingCirclePrivate() {}

QGeoBoundingCirclePrivate& QGeoBoundingCirclePrivate::operator= (const QGeoBoundingCirclePrivate & other)
{
    center = other.center;
    radius = other.radius;

    return *this;
}

bool QGeoBoundingCirclePrivate::operator== (const QGeoBoundingCirclePrivate &other) const
{
    return ((center == other.center) && (radius == other.radius));
}

QTM_END_NAMESPACE

