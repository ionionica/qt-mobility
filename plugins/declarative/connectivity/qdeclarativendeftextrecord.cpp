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

#include "qdeclarativendeftextrecord_p.h"

#include <QtCore/QLocale>

/*!
    \qmlclass NdefTextRecord QDeclarativeNdefTextRecord
    \brief The NdefTextRecord element represents an NFC RTD-Text NDEF record.

    \ingroup connectivity-qml
    \inmodule QtConnectivity
    \since Mobility 1.2

    \inherits NdefRecord

    \sa QNdefNfcTextRecord

    The NdefTextRecord element is part of the \bold {QtMobility.connectivity 1.2} module.

    The NdefTextRecord element contains a localized piece of text that can be display to the user.
    An NDEF message may contain many text records for different locales, it is up to the
    application to select the most appropriate one to display to the user.  The localeMatch
    property can be used to determine if the text record has been matched.
*/

/*!
    \qmlproperty string NdefTextRecord::text
    \since Mobility 1.2

    This property holds the text which should be displayed when the current locale matches
    \l locale.
*/

/*!
    \qmlproperty string NdefTextRecord::locale
    \since Mobility 1.2

    This property holds the locale that this text record is for.
*/

/*!
    \qmlproperty enumeration NdefTextRecord::localeMatch
    \since Mobility 1.2

    This property holds an enum describing how closely the locale of the text record matches the
    applications current locale.  The application should display only the text record that most
    closely matches the applications current locale.

    \table
        \header
            \o Value
            \o Description
        \row
            \o LocaleMatchedNone
            \o The text record does not match at all.
        \row
            \o LocaleMatchedEnglish
            \o The language of the text record is English and the language of application's current
               locale is \bold {not} English.  The English language text should be displayed if
               there is not a more appropriate match.
        \row
            \o LocaleMatchedLanguage
            \o The language of the text record and the language of the applications's current
               locale are the same.
        \row
            \o LocaleMatchedLanguageAndCountry
            \o The language and country of the text record matches that of the applicatin's current
               locale.
    \endtable
*/

Q_DECLARE_NDEFRECORD(QDeclarativeNdefTextRecord, QNdefRecord::NfcRtd, "T")

QDeclarativeNdefTextRecord::QDeclarativeNdefTextRecord(QObject *parent)
:   QDeclarativeNdefRecord(QNdefNfcTextRecord(), parent)
{
}

QDeclarativeNdefTextRecord::QDeclarativeNdefTextRecord(const QNdefRecord &record, QObject *parent)
:   QDeclarativeNdefRecord(QNdefNfcTextRecord(record), parent)
{
}

QDeclarativeNdefTextRecord::~QDeclarativeNdefTextRecord()
{
}

QString QDeclarativeNdefTextRecord::text() const
{
    QNdefNfcTextRecord textRecord(record());

    return textRecord.text();
}

void QDeclarativeNdefTextRecord::setText(const QString &text)
{
    QNdefNfcTextRecord textRecord(record());

    if (textRecord.text() == text)
        return;

    textRecord.setText(text);
    setRecord(textRecord);
    emit textChanged();
}

QString QDeclarativeNdefTextRecord::locale() const
{
    if (!record().isRecordType<QNdefNfcTextRecord>())
        return QString();

    QNdefNfcTextRecord textRecord(record());

    return textRecord.locale();
}

void QDeclarativeNdefTextRecord::setLocale(const QString &locale)
{
    QNdefNfcTextRecord textRecord(record());

    if (textRecord.locale() == locale)
        return;

    LocaleMatch previous = localeMatch();

    textRecord.setLocale(locale);
    setRecord(textRecord);
    emit localeChanged();

    if (previous != localeMatch())
        emit localeMatchChanged();
}

QDeclarativeNdefTextRecord::LocaleMatch QDeclarativeNdefTextRecord::localeMatch() const
{
    const QLocale recordLocale(locale());
    const QLocale defaultLocale;

    if (recordLocale == defaultLocale)
        return LocaleMatchedLanguageAndCountry;
    else if (recordLocale.language() == defaultLocale.language())
        return LocaleMatchedLanguage;
    else if (recordLocale.language() == QLocale::English)
        return LocaleMatchedEnglish;
    else
        return LocaleMatchedNone;
}
