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
#ifndef CDATABASEMANAGERSERVER_H_
#define CDATABASEMANAGERSERVER_H_

#include <qmobilityglobal.h>
#include <e32base.h>
#include <QObject>
#include "databasemanagerserver.pan"

class QFileSystemWatcher;

//QTM_BEGIN_NAMESPACE
namespace QtMobility {

class DatabaseManagerServerSignalHandler;
class ServiceDatabase;

  
// needed for creating server thread.
const TUint KDefaultHeapSize = 0x10000;

class CDatabaseManagerServer : public CServer2
    {
    public:
        CDatabaseManagerServer();
        CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;

    public:
        static void PanicServer(TDatabaseManagerSerververPanic aPanic);        
        
        void IncreaseSessions();
        void DecreaseSessions();
        
        void Shutdown();
        
        void DiscoverServices();
        
    private:
        void initDbPath();        
        
        QStringList getExistingDbfiles(const QString &dirPath);
        void mergeDbFiles(const QString &dbDirPath, const QString &dbFileName);
        
        static const TInt timeoutInterval;
        int iSessionCount;        
        CPeriodic *iPeriodic;
        ServiceDatabase *iDb;
        QFileSystemWatcher *iWatcher;
        DatabaseManagerServerSignalHandler *iDatabaseManagerServerSignalHandler;
    };


class DatabaseManagerServerSignalHandler : public QObject
    {
    Q_OBJECT
    
    public:
        DatabaseManagerServerSignalHandler(CDatabaseManagerServer *databaseManagerServerSession) 
        {
          iDatabaseManagerServerSession = databaseManagerServerSession;
        }

    public Q_SLOTS:
        void importChanged(const QString &path);
        
    public:
        CDatabaseManagerServer *iDatabaseManagerServerSession;
    };

//QTM_END_NAMESPACE
}

#endif

// End of File
