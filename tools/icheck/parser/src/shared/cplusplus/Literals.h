/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/
// Copyright (c) 2008 Roberto Raggi <roberto.raggi@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef CPLUSPLUS_LITERALS_H
#define CPLUSPLUS_LITERALS_H

#include "CPlusPlusForwardDeclarations.h"
#include "Token.h"


namespace CPlusPlus {

class CPLUSPLUS_EXPORT Literal
{
    Literal(const Literal &other);
    void operator =(const Literal &other);

public:
    typedef const char *iterator;
    typedef iterator const_iterator;

public:
    Literal(const char *chars, unsigned size);
    virtual ~Literal();

    iterator begin() const;
    iterator end() const;

    char at(unsigned index) const;
    const char *chars() const;
    unsigned size() const;

    unsigned hashCode() const;
    static unsigned hashCode(const char *chars, unsigned size);

    bool isEqualTo(const Literal *other) const;

private:
    char *_chars;
    unsigned _size;
    unsigned _hashCode;

public:
    // ### private
    unsigned _index;
    Literal *_next;
};

class CPLUSPLUS_EXPORT StringLiteral: public Literal
{
public:
    StringLiteral(const char *chars, unsigned size);
    virtual ~StringLiteral();
};

class CPLUSPLUS_EXPORT NumericLiteral: public Literal
{
public:
    NumericLiteral(const char *chars, unsigned size);
    virtual ~NumericLiteral();

    bool isChar() const;
    bool isWideChar() const;
    bool isInt() const;
    bool isFloat() const;
    bool isDouble() const;
    bool isLongDouble() const;
    bool isLong() const;
    bool isLongLong() const;

    bool isUnsigned() const;
    bool isHex() const;

private:
    struct Flags {
        unsigned _type      : 8;
        unsigned _isHex     : 1;
        unsigned _isUnsigned: 1;
    };
    union {
        unsigned _flags;
        Flags f;
    };
};

class CPLUSPLUS_EXPORT Identifier: public Literal
{
public:
    Identifier(const char *chars, unsigned size);
    virtual ~Identifier();
};

} // end of namespace CPlusPlus


#endif // CPLUSPLUS_LITERALS_H