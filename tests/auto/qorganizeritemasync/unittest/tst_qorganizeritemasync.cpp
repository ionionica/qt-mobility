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

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QScopedPointer>

#include "qtorganizer.h"
#include "qorganizeritemmanagerdataholder.h" //QOrganizerItemManagerDataHolder

QTM_USE_NAMESPACE
/* Define an innocuous request (fetch ie doesn't mutate) to "fill up" any queues */
#define FILL_QUEUE_WITH_FETCH_REQUESTS() QOrganizerItemFetchRequest fqifr1, fqifr2, fqifr3; \
                                         QOrganizerItemDetailDefinitionFetchRequest fqdfr1, fqdfr2, fqdfr3; \
                                         fqifr1.start(); \
                                         fqifr2.start(); \
                                         fqifr3.start(); \
                                         fqdfr1.start(); \
                                         fqdfr2.start(); \
                                         fqdfr3.start();

//TESTED_COMPONENT=src/contacts
//TESTED_CLASS=
//TESTED_FILES=

// Unfortunately the plumbing isn't in place to allow cancelling requests at arbitrary points
// in their processing.  So we do multiple loops until things work out.. or not
#define MAX_OPTIMISTIC_SCHEDULING_LIMIT 100


// Thread capable QThreadSignalSpy (to avoid data races with count/appendArgS)
class QThreadSignalSpy: public QObject
{
public:
    QThreadSignalSpy(QObject *obj, const char *aSignal)
    {
        QMutexLocker m(&lock);
#ifdef Q_CC_BOR
        const int memberOffset = QObject::staticMetaObject.methodCount();
#else
        static const int memberOffset = QObject::staticMetaObject.methodCount();
#endif
        Q_ASSERT(obj);
        Q_ASSERT(aSignal);

        if (((aSignal[0] - '0') & 0x03) != QSIGNAL_CODE) {
            qWarning("QThreadSignalSpy: Not a valid signal, use the SIGNAL macro");
            return;
        }

        QByteArray ba = QMetaObject::normalizedSignature(aSignal + 1);
        const QMetaObject *mo = obj->metaObject();
        int sigIndex = mo->indexOfMethod(ba.constData());
        if (sigIndex < 0) {
            qWarning("QThreadSignalSpy: No such signal: '%s'", ba.constData());
            return;
        }

        if (!QMetaObject::connect(obj, sigIndex, this, memberOffset,
                    Qt::DirectConnection, 0)) {
            qWarning("QThreadSignalSpy: QMetaObject::connect returned false. Unable to connect.");
            return;
        }
        sig = ba;
        initArgs(mo->method(sigIndex));
    }

    inline bool isValid() const { return !sig.isEmpty(); }
    inline QByteArray signal() const { return sig; }

    int qt_metacall(QMetaObject::Call call, int methodId, void **a)
    {
        methodId = QObject::qt_metacall(call, methodId, a);
        if (methodId < 0)
            return methodId;

        if (call == QMetaObject::InvokeMetaMethod) {
            if (methodId == 0) {
                appendArgs(a);
            }
            --methodId;
        }
        return methodId;
    }

    // The QList<QVariantList> API we actually use
    int count() const
    {
        QMutexLocker m(&lock);
        return savedArgs.count();
    }
    void clear()
    {
        QMutexLocker m(&lock);
        savedArgs.clear();
    }

private:
    void initArgs(const QMetaMethod &member)
    {
        QList<QByteArray> params = member.parameterTypes();
        for (int i = 0; i < params.count(); ++i) {
            int tp = QMetaType::type(params.at(i).constData());
            if (tp == QMetaType::Void)
                qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                         params.at(i).constData());
            args << tp;
        }
    }

    void appendArgs(void **a)
    {
        QMutexLocker m(&lock);
        QList<QVariant> list;
        for (int i = 0; i < args.count(); ++i) {
            QMetaType::Type type = static_cast<QMetaType::Type>(args.at(i));
            list << QVariant(type, a[i + 1]);
        }
        savedArgs.append(list);
    }

    // the full, normalized signal name
    QByteArray sig;
    // holds the QMetaType types for the argument list of the signal
    QList<int> args;

    mutable QMutex lock;
    // Different API
    QList< QVariantList> savedArgs;
};

class tst_QOrganizerItemAsync : public QObject
{
    Q_OBJECT

public:
    tst_QOrganizerItemAsync();
    virtual ~tst_QOrganizerItemAsync();

public slots:
    void initTestCase();
    void cleanupTestCase();

private:
    void addManagers(QStringList includes = QStringList()); // add standard managers to the data

private slots:
    void testDestructor();
    void testDestructor_data() { addManagers(QStringList(QString("maliciousplugin"))); }

    void itemFetch();
    void itemFetch_data() { addManagers(); }
    void itemFetchById();
    void itemFetchById_data() { addManagers(); }
    void itemIdFetch();
    void itemIdFetch_data() { addManagers(); }
    void itemRemove();
    void itemRemove_data() { addManagers(); }
    void itemSave();
    void itemSave_data() { addManagers(); }
    void itemPartialSave();
    void itemPartialSave_data() { addManagers(); }

    void definitionFetch();
    void definitionFetch_data() { addManagers(); }
    void definitionRemove();
    void definitionRemove_data() { addManagers(); }
    void definitionSave();
    void definitionSave_data() { addManagers(); }

    void collectionFetch();
    void collectionFetch_data() { addManagers(); }
    void collectionIdFetch();
    void collectionIdFetch_data() { addManagers(); }
    void collectionRemove();
    void collectionRemove_data() { addManagers(); }
    void collectionSave();
    void collectionSave_data() { addManagers(); }

    void testQuickDestruction();
    void testQuickDestruction_data() { addManagers(QStringList(QString("maliciousplugin"))); }

    void threadDelivery();
    void threadDelivery_data() { addManagers(QStringList(QString("maliciousplugin"))); }
protected slots:
    void resultsAvailableReceived();
    void deleteRequest();

private:
    bool compareItemLists(QList<QOrganizerItem> lista, QList<QOrganizerItem> listb);
    bool compareItems(QOrganizerItem ca, QOrganizerItem cb);
    bool containsIgnoringTimestamps(const QList<QOrganizerItem>& list, const QOrganizerItem& c);
    bool compareIgnoringTimestamps(const QOrganizerItem& ca, const QOrganizerItem& cb);
    QOrganizerItemManager* prepareModel(const QString& uri);

    Qt::HANDLE m_mainThreadId;
    Qt::HANDLE m_resultsAvailableSlotThreadId;
    QScopedPointer<QOrganizerItemManagerDataHolder> managerDataHolder;
};

tst_QOrganizerItemAsync::tst_QOrganizerItemAsync()
{
    // ensure we can load all of the plugins we need to.
    QString path = QApplication::applicationDirPath() + "/dummyplugin/plugins";
    QApplication::addLibraryPath(path);

    qRegisterMetaType<QOrganizerItemAbstractRequest::State>("QOrganizerItemAbstractRequest::State");
}

tst_QOrganizerItemAsync::~tst_QOrganizerItemAsync()
{
}

void tst_QOrganizerItemAsync::initTestCase()
{
    managerDataHolder.reset(new QOrganizerItemManagerDataHolder());
}

void tst_QOrganizerItemAsync::cleanupTestCase()
{
    managerDataHolder.reset(0);
}

bool tst_QOrganizerItemAsync::compareItemLists(QList<QOrganizerItem> lista, QList<QOrganizerItem> listb)
{
    // NOTE: This compare is contact order insensitive.  
    
    // Remove matching contacts
    foreach (QOrganizerItem a, lista) {
        foreach (QOrganizerItem b, listb) {
            if (compareItems(a, b)) {
                lista.removeOne(a);
                listb.removeOne(b);
                break;
            }
        }
    }    
    return (lista.count() == 0 && listb.count() == 0);
}

bool tst_QOrganizerItemAsync::compareItems(QOrganizerItem ca, QOrganizerItem cb)
{
    // NOTE: This compare is contact detail order insensitive.
    
    if (ca.localId() != cb.localId())
        return false;
    
    QList<QOrganizerItemDetail> aDetails = ca.details();
    QList<QOrganizerItemDetail> bDetails = cb.details();

    // Remove matching details
    foreach (QOrganizerItemDetail ad, aDetails) {
        foreach (QOrganizerItemDetail bd, bDetails) {
            if (ad == bd) {
                ca.removeDetail(&ad);
                cb.removeDetail(&bd);
                break;
            }
            
            // Special handling for timestamp
            if (ad.definitionName() == QOrganizerItemTimestamp::DefinitionName &&
                bd.definitionName() == QOrganizerItemTimestamp::DefinitionName) {
                QOrganizerItemTimestamp at = static_cast<QOrganizerItemTimestamp>(ad);
                QOrganizerItemTimestamp bt = static_cast<QOrganizerItemTimestamp>(bd);
                if (at.created().toString() == bt.created().toString() &&
                    at.lastModified().toString() == bt.lastModified().toString()) {
                    ca.removeDetail(&ad);
                    cb.removeDetail(&bd);
                    break;
                }
                    
            }            
        }
    }
    return (ca == cb);
}

bool tst_QOrganizerItemAsync::containsIgnoringTimestamps(const QList<QOrganizerItem>& list, const QOrganizerItem& c)
{
    QList<QOrganizerItem> cl = list;
    QOrganizerItem a(c);
    for (int i = 0; i < cl.size(); i++) {
        QOrganizerItem b(cl.at(i));
        if (compareIgnoringTimestamps(a, b))
            return true;
    }

    return false;
}

bool tst_QOrganizerItemAsync::compareIgnoringTimestamps(const QOrganizerItem& ca, const QOrganizerItem& cb)
{
    // Compares two contacts, ignoring any timestamp details
    QOrganizerItem a(ca);
    QOrganizerItem b(cb);
    QList<QOrganizerItemDetail> aDetails = a.details();
    QList<QOrganizerItemDetail> bDetails = b.details();

    // They can be in any order, so loop
    // First remove any matches, and any timestamps
    foreach (QOrganizerItemDetail d, aDetails) {
        foreach (QOrganizerItemDetail d2, bDetails) {
            if (d == d2) {
                a.removeDetail(&d);
                b.removeDetail(&d2);
                break;
            }

            if (d.definitionName() == QOrganizerItemTimestamp::DefinitionName) {
                a.removeDetail(&d);
            }

            if (d2.definitionName() == QOrganizerItemTimestamp::DefinitionName) {
                b.removeDetail(&d2);
            }
        }
    }

    if (a == b)
        return true;
    return false;
}

void tst_QOrganizerItemAsync::testDestructor()
{
    QFETCH(QString, uri);
    QOrganizerItemManager* cm = prepareModel(uri);
    QOrganizerItemFetchRequest* req = new QOrganizerItemFetchRequest;
    req->setManager(cm);

    QOrganizerItemManager* cm2 = prepareModel(uri);
    QOrganizerItemFetchRequest* req2 = new QOrganizerItemFetchRequest;
    req2->setManager(cm2);

    // first, delete manager then request
    delete cm;
    delete req;

    // second, delete request then manager
    delete req2;
    delete cm2;
}

void tst_QOrganizerItemAsync::deleteRequest()
{
    // Delete the sender (request) - check that it doesn't crash in this common coding error
    delete sender();
}

void tst_QOrganizerItemAsync::itemFetch()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));

    QOrganizerItemFetchRequest ifr;
    QVERIFY(ifr.type() == QOrganizerItemAbstractRequest::ItemFetchRequest);

    // initial state - not started, no manager.
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.start());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());

    // "all contacts" retrieval
    QOrganizerItemFilter fil;
    ifr.setManager(oim.data());
    QCOMPARE(ifr.manager(), oim.data());
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());
    qRegisterMetaType<QOrganizerItemFetchRequest*>("QOrganizerItemFetchRequest*");
    QThreadSignalSpy spy(&ifr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    ifr.setFilter(fil);
    QCOMPARE(ifr.filter(), fil);
    QVERIFY(!ifr.cancel()); // not started

    QVERIFY(ifr.start());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerItemLocalId> itemIds = oim->itemIds();
    QList<QOrganizerItem> items = ifr.items();
    QCOMPARE(itemIds.size(), items.size());
    for (int i = 0; i < itemIds.size(); i++) {
        QOrganizerItem curr = oim->item(itemIds.at(i));
        QVERIFY(items.at(i) == curr);
    }

    // asynchronous detail filtering
    QOrganizerItemDetailFilter dfil;
    dfil.setDetailDefinitionName(QOrganizerItemLocation::DefinitionName, QOrganizerItemLocation::FieldLocationName);
    ifr.setFilter(dfil);
    QVERIFY(ifr.filter() == dfil);
    QVERIFY(!ifr.cancel()); // not started

    QVERIFY(ifr.start());
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    itemIds = oim->itemIds(dfil);
    items = ifr.items();
    QCOMPARE(itemIds.size(), items.size());
    for (int i = 0; i < itemIds.size(); i++) {
        QOrganizerItem curr = oim->item(itemIds.at(i));
        QVERIFY(items.at(i) == curr);
    }

    // sort order
    QOrganizerItemSortOrder sortOrder;
    sortOrder.setDetailDefinitionName(QOrganizerItemPriority::DefinitionName, QOrganizerItemPriority::FieldPriority);
    QList<QOrganizerItemSortOrder> sorting;
    sorting.append(sortOrder);
    ifr.setFilter(fil);
    ifr.setSorting(sorting);
    QCOMPARE(ifr.sorting(), sorting);
    QVERIFY(!ifr.cancel()); // not started
    QVERIFY(ifr.start());
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    itemIds = oim->itemIds(sorting);
    items = ifr.items();
    QCOMPARE(itemIds.size(), items.size());
    for (int i = 0; i < itemIds.size(); i++) {
        QOrganizerItem curr = oim->item(itemIds.at(i));
        QVERIFY(items.at(i) == curr);
    }

    // restrictions
    sorting.clear();
    ifr.setFilter(fil);
    ifr.setSorting(sorting);
    QOrganizerItemFetchHint fetchHint;
    fetchHint.setDetailDefinitionsHint(QStringList(QOrganizerItemDescription::DefinitionName));
    ifr.setFetchHint(fetchHint);
    QCOMPARE(ifr.fetchHint().detailDefinitionsHint(), QStringList(QOrganizerItemDescription::DefinitionName));
    QVERIFY(!ifr.cancel()); // not started
    QVERIFY(ifr.start());
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    itemIds = oim->itemIds(sorting);
    items = ifr.items();
    QCOMPARE(itemIds.size(), items.size());
    for (int i = 0; i < itemIds.size(); i++) {
        // create a contact from the restricted data only (id + display label)
        QOrganizerItem currFull = oim->item(itemIds.at(i));
        QOrganizerItem currRestricted;
        currRestricted.setId(currFull.id());
        QList<QOrganizerItemDescription> descriptions = currFull.details<QOrganizerItemDescription>();
        foreach (const QOrganizerItemDescription& description, descriptions) {
            QOrganizerItemDescription descr = description;
            if (!descr.isEmpty()) {
                currRestricted.saveDetail(&descr);
            }
        }

        // now find the contact in the retrieved list which our restricted contact mimics
        QOrganizerItem retrievedRestricted;
        bool found = false;
        foreach (const QOrganizerItem& retrieved, items) {
            if (retrieved.id() == currRestricted.id()) {
                retrievedRestricted = retrieved;
                found = true;
            }
        }

        QVERIFY(found); // must exist or fail.

        // ensure that the contact is the same (except synth fields)
        QList<QOrganizerItemDetail> retrievedDetails = retrievedRestricted.details();
        QList<QOrganizerItemDetail> expectedDetails = currRestricted.details();
        foreach (const QOrganizerItemDetail& det, expectedDetails) {
            // ignore backend synthesised details
            // again, this requires a "default contact details" function to work properly.
            if (det.definitionName() == QOrganizerItemDisplayLabel::DefinitionName
                || det.definitionName() == QOrganizerItemTimestamp::DefinitionName) {
                continue;
            }

            // everything else in the expected contact should be in the retrieved one.
            QVERIFY(retrievedDetails.contains(det));
        }
    }

    // cancelling
    sorting.clear();
    ifr.setFilter(fil);
    ifr.setSorting(sorting);
    ifr.setFetchHint(QOrganizerItemFetchHint());

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!ifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(ifr.start());
        if (!ifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            spy.clear();
            ifr.waitForFinished();
            sorting.clear();
            ifr.setFilter(fil);
            ifr.setSorting(sorting);
            ifr.setFetchHint(QOrganizerItemFetchHint());
            ifr.setFetchHint(QOrganizerItemFetchHint());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(ifr.waitForFinished());
        QVERIFY(ifr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!ifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(ifr.start());
        if (!ifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            ifr.waitForFinished();
            sorting.clear();
            ifr.setFilter(fil);
            ifr.setSorting(sorting);
            ifr.setFetchHint(QOrganizerItemFetchHint());
            bailoutCount -= 1;
            spy.clear();
            if (!bailoutCount) {
                //qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }
        ifr.waitForFinished();
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        QVERIFY(!ifr.isActive());
        QVERIFY(ifr.state() == QOrganizerItemAbstractRequest::CanceledState);
        break;
    }

    // Now test deletion in the first slot called
    QOrganizerItemFetchRequest *ifr2 = new QOrganizerItemFetchRequest();
    QPointer<QObject> obj(ifr2);
    ifr2->setManager(oim.data());
    connect(ifr2, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)), this, SLOT(deleteRequest()));
    QVERIFY(ifr2->start());
    int i = 100;
    // at this point we can't even call wait for finished..
    while(obj && i > 0) {
        QTest::qWait(50); // force it to process events at least once.
        i--;
    }
    QVERIFY(obj == NULL);
}

void tst_QOrganizerItemAsync::itemFetchById()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));

/* XXX TODO: fetchbyid request for items as well as contacts!!!

    QOrganizerItemFetchByIdRequest ifr;
    QVERIFY(ifr.type() == QOrganizerItemAbstractRequest::ItemFetchByIdRequest);

    // initial state - not started, no manager.
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.start());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());

    // get all contact ids
    QList<QOrganizerItemLocalId> itemIds(oim->itemIds());

    // "all contacts" retrieval
    ifr.setManager(oim.data());
    ifr.setLocalIds(itemIds);
    QCOMPARE(ifr.manager(), oim.data());
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());
    qRegisterMetaType<QOrganizerItemFetchByIdRequest*>("QOrganizerItemFetchByIdRequest*");
    QThreadSignalSpy spy(&ifr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(!ifr.cancel()); // not started

    QVERIFY(ifr.start());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerItem> items = ifr.items();
    QCOMPARE(itemIds.size(), items.size());
    for (int i = 0; i < itemIds.size(); i++) {
        QOrganizerItem curr = oim->item(itemIds.at(i));
        QVERIFY(items.at(i) == curr);
    }
XXX TODO: fetchbyid request for items as well as contacts!!! */
}


void tst_QOrganizerItemAsync::itemIdFetch()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerItemLocalIdFetchRequest ifr;
    QVERIFY(ifr.type() == QOrganizerItemAbstractRequest::ItemLocalIdFetchRequest);

    // initial state - not started, no manager.
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.start());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());

    // "all contacts" retrieval
    QOrganizerItemFilter fil;
    ifr.setManager(oim.data());
    QCOMPARE(ifr.manager(), oim.data());
    QVERIFY(!ifr.isActive());
    QVERIFY(!ifr.isFinished());
    QVERIFY(!ifr.cancel());
    QVERIFY(!ifr.waitForFinished());
    qRegisterMetaType<QOrganizerItemLocalIdFetchRequest*>("QOrganizerItemLocalIdFetchRequest*");

    QThreadSignalSpy spy(&ifr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    ifr.setFilter(fil);
    QCOMPARE(ifr.filter(), fil);
    QVERIFY(!ifr.cancel()); // not started
    QVERIFY(ifr.start());

    QVERIFY((ifr.isActive() &&ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerItemLocalId> itemIds = oim->itemIds();
    QList<QOrganizerItemLocalId> result = ifr.itemIds();
    QCOMPARE(itemIds, result);

    // asynchronous detail filtering
    QOrganizerItemDetailFilter dfil;
    dfil.setDetailDefinitionName(QOrganizerItemLocation::DefinitionName, QOrganizerItemLocation::FieldLocationName);
    ifr.setFilter(dfil);
    QVERIFY(ifr.filter() == dfil);
    QVERIFY(!ifr.cancel()); // not started

    QVERIFY(ifr.start());
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    itemIds = oim->itemIds(dfil);
    result = ifr.itemIds();
    QCOMPARE(itemIds, result);

    // sort order
    QOrganizerItemSortOrder sortOrder;
    sortOrder.setDetailDefinitionName(QOrganizerItemPriority::DefinitionName, QOrganizerItemPriority::FieldPriority);
    QList<QOrganizerItemSortOrder> sorting;
    sorting.append(sortOrder);
    ifr.setFilter(fil);
    ifr.setSorting(sorting);
    QCOMPARE(ifr.sorting(), sorting);
    QVERIFY(!ifr.cancel()); // not started
    QVERIFY(ifr.start());
    QVERIFY((ifr.isActive() && ifr.state() == QOrganizerItemAbstractRequest::ActiveState) || ifr.isFinished());
    //QVERIFY(ifr.isFinished() || !ifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(ifr.waitForFinished());
    QVERIFY(ifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    itemIds = oim->itemIds(sorting);
    result = ifr.itemIds();
    QCOMPARE(itemIds, result);

    // cancelling
    sorting.clear();
    ifr.setFilter(fil);
    ifr.setSorting(sorting);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!ifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(ifr.start());
        if (!ifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            ifr.waitForFinished();
            sorting.clear();
            ifr.setFilter(fil);
            ifr.setSorting(sorting);
            bailoutCount -= 1;
            spy.clear();
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(ifr.waitForFinished());
        QVERIFY(ifr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!ifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(ifr.start());
        if (!ifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            ifr.waitForFinished();
            sorting.clear();
            ifr.setFilter(fil);
            ifr.setSorting(sorting);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }
        ifr.waitForFinished();
        QVERIFY(ifr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

}

void tst_QOrganizerItemAsync::itemRemove()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerItemRemoveRequest irr;
    QVERIFY(irr.type() == QOrganizerItemAbstractRequest::ItemRemoveRequest);

    // initial state - not started, no manager.
    QVERIFY(!irr.isActive());
    QVERIFY(!irr.isFinished());
    QVERIFY(!irr.start());
    QVERIFY(!irr.cancel());
    QVERIFY(!irr.waitForFinished());

    // fill manager with test data
    QOrganizerTodo testTodo1;
    QOrganizerItemDisplayLabel label;
    label.setLabel("Test todo 1");
    testTodo1.saveDetail(&label);
    QVERIFY(oim->saveItem(&testTodo1));

    testTodo1.setId(QOrganizerItemId());
    label.setLabel("Test todo 2");
    testTodo1.saveDetail(&label);
    QOrganizerItemComment comment;
    comment.setComment("todo comment");
    testTodo1.saveDetail(&comment);
    QVERIFY(oim->saveItem(&testTodo1));

    QList<QOrganizerItemLocalId> allIds(oim->itemIds());
    QVERIFY(!allIds.isEmpty());
    QOrganizerItemLocalId removableId(allIds.first());

    // specific contact set
    irr.setItemId(removableId);
    QVERIFY(irr.itemIds() == QList<QOrganizerItemLocalId>() << removableId);

    // specific contact removal via detail filter
    int originalCount = oim->itemIds().size();
    QOrganizerItemDetailFilter dfil;
    dfil.setDetailDefinitionName(QOrganizerItemComment::DefinitionName, QOrganizerItemComment::FieldComment);
    irr.setItemIds(oim->itemIds(dfil));
    irr.setManager(oim.data());
    QCOMPARE(irr.manager(), oim.data());
    QVERIFY(!irr.isActive());
    QVERIFY(!irr.isFinished());
    QVERIFY(!irr.cancel());
    QVERIFY(!irr.waitForFinished());
    qRegisterMetaType<QOrganizerItemRemoveRequest*>("QOrganizerItemRemoveRequest*");
    QThreadSignalSpy spy(&irr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(!irr.cancel()); // not started

    QVERIFY(!oim->itemIds(dfil).isEmpty());

    QVERIFY(irr.start());

    QVERIFY((irr.isActive() &&irr.state() == QOrganizerItemAbstractRequest::ActiveState) || irr.isFinished());
    //QVERIFY(irr.isFinished() || !irr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(irr.waitForFinished());
    QVERIFY(irr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(oim->itemIds().size(), originalCount - 1);
    QVERIFY(oim->itemIds(dfil).isEmpty());

    // remove all contacts
    dfil.setDetailDefinitionName(QOrganizerItemDisplayLabel::DefinitionName); // delete everything.
    irr.setItemIds(oim->itemIds(dfil));
    
    QVERIFY(!irr.cancel()); // not started
    QVERIFY(irr.start());

    QVERIFY((irr.isActive() && irr.state() == QOrganizerItemAbstractRequest::ActiveState) || irr.isFinished());
    //QVERIFY(irr.isFinished() || !irr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(irr.waitForFinished());
    QVERIFY(irr.isFinished());

    QCOMPARE(oim->itemIds().size(), 0); // no contacts should be left.
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    // cancelling
    QOrganizerItem temp;
    QOrganizerItemDescription description;
    description.setDescription("Should not be removed");
    temp.saveDetail(&description);
    oim->saveItem(&temp);
    irr.setItemIds(oim->itemIds(dfil));

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!irr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(spy.count() == 0);
        QVERIFY(irr.start());
        if (!irr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            irr.waitForFinished();
            irr.setItemIds(oim->itemIds(dfil));
            temp.setId(QOrganizerItemId());
            if (!oim->saveItem(&temp)) {
                QSKIP("Unable to save temporary contact for remove request cancellation test!", SkipSingle);
            }
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(irr.waitForFinished());
        QVERIFY(irr.isCanceled());
        QCOMPARE(oim->itemIds().size(), 1);
        QCOMPARE(oim->itemIds(), irr.itemIds());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!irr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(irr.start());
        if (!irr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            irr.waitForFinished();
            irr.setItemIds(oim->itemIds(dfil));
            temp.setId(QOrganizerItemId());
            oim->saveItem(&temp);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        irr.waitForFinished();
        QVERIFY(irr.isCanceled());
        QCOMPARE(oim->itemIds().size(), 1);
        QCOMPARE(oim->itemIds(), irr.itemIds());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

}

void tst_QOrganizerItemAsync::itemSave()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerItemSaveRequest isr;
    QVERIFY(isr.type() == QOrganizerItemAbstractRequest::ItemSaveRequest);

    // initial state - not started, no manager.
    QVERIFY(!isr.isActive());
    QVERIFY(!isr.isFinished());
    QVERIFY(!isr.start());
    QVERIFY(!isr.cancel());
    QVERIFY(!isr.waitForFinished());

    // save a new item
    int originalCount = oim->itemIds().size();
    QOrganizerTodo testTodo;
    QOrganizerItemDescription description;
    description.setDescription("Test todo");
    testTodo.saveDetail(&description);
    QList<QOrganizerItem> saveList;
    saveList << testTodo;
    isr.setManager(oim.data());
    QCOMPARE(isr.manager(), oim.data());
    QVERIFY(!isr.isActive());
    QVERIFY(!isr.isFinished());
    QVERIFY(!isr.cancel());
    QVERIFY(!isr.waitForFinished());
    qRegisterMetaType<QOrganizerItemSaveRequest*>("QOrganizerItemSaveRequest*");
    QThreadSignalSpy spy(&isr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    isr.setItem(testTodo);
    QCOMPARE(isr.items(), saveList);
    QVERIFY(!isr.cancel()); // not started
    QVERIFY(isr.start());

    QVERIFY((isr.isActive() && isr.state() == QOrganizerItemAbstractRequest::ActiveState) || isr.isFinished());
    //QVERIFY(isr.isFinished() || !isr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(isr.waitForFinished());
    QVERIFY(isr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerItem> expected = isr.items();
    QCOMPARE(expected.size(), 1);
    QList<QOrganizerItem> result;
    result << oim->item(expected.first().id().localId());
    //some backends add extra fields, so this doesn't work:
    //QCOMPARE(result, expected);
    // XXX: really, we should use isSuperset() from tst_QOrganizerItemManager, but this will do for now:
    QVERIFY(result.first().detail<QOrganizerItemDescription>() == description);
    QCOMPARE(oim->itemIds().size(), originalCount + 1);

    // update a previously saved contact
    QOrganizerItemPriority priority;
    priority.setPriority(QOrganizerItemPriority::LowestPriority);
    testTodo = result.first();
    testTodo.saveDetail(&priority);
    saveList.clear();
    saveList << testTodo;
    isr.setItems(saveList);
    QCOMPARE(isr.items(), saveList);
    QVERIFY(!isr.cancel()); // not started
    QVERIFY(isr.start());

    QVERIFY((isr.isActive() && isr.state() == QOrganizerItemAbstractRequest::ActiveState) || isr.isFinished());
    //QVERIFY(isr.isFinished() || !isr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(isr.waitForFinished());

    QVERIFY(isr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    expected = isr.items();
    result.clear();
    result << oim->item(expected.first().id().localId());
    //QVERIFY(compareContactLists(result, expected));

    //here we can't compare the whole contact details, testTodo would be updated by async call because we just use QThreadSignalSpy to receive signals.
    //QVERIFY(containsIgnoringTimestamps(result, testTodo));
    // XXX: really, we should use isSuperset() from tst_QOrganizerItemManager, but this will do for now:
    QVERIFY(result.first().detail<QOrganizerItemPriority>().priority() == priority.priority());
    
    QCOMPARE(oim->itemIds().size(), originalCount + 1);

    // cancelling
    QOrganizerItem temp = testTodo;
    temp.setDisplayLabel("should not get saved");
    saveList.clear();
    saveList << temp;
    isr.setItems(saveList);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!isr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(isr.start());
        if (!isr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            isr.waitForFinished();
            saveList = isr.items();
            if (oim->itemIds().size() > (originalCount + 1) && !oim->removeItem(saveList.at(0).localId())) {
                QSKIP("Unable to remove saved contact to test cancellation of contact save request", SkipSingle);
            }
            saveList.clear();
            saveList << temp;
            isr.setItems(saveList);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(isr.waitForFinished());
        QVERIFY(isr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        expected.clear();
        QList<QOrganizerItemLocalId> allItems = oim->itemIds();
        for (int i = 0; i < allItems.size(); i++) {
            expected.append(oim->item(allItems.at(i)));
        }
        QVERIFY(!expected.contains(temp));
        QCOMPARE(oim->itemIds().size(), originalCount + 1);
        break;
    }
    // restart, and wait for progress after cancel.

    while (true) {
        QVERIFY(!isr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(isr.start());
        if (!isr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            isr.waitForFinished();
            saveList = isr.items();
            if (oim->itemIds().size() > (originalCount + 1) && !oim->removeItem(saveList.at(0).localId())) {
                QSKIP("Unable to remove saved contact to test cancellation of contact save request", SkipSingle);
            }
            saveList.clear();
            saveList << temp;
            isr.setItems(saveList);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        isr.waitForFinished(); // now wait until finished (if it hasn't already).
        QVERIFY(isr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        expected.clear();
        QList<QOrganizerItemLocalId> allItems = oim->itemIds();
        for (int i = 0; i < allItems.size(); i++) {
            expected.append(oim->item(allItems.at(i)));
        }
        QVERIFY(!expected.contains(temp));
        QCOMPARE(oim->itemIds().size(), originalCount + 1);
        break;
    }
}

void tst_QOrganizerItemAsync::itemPartialSave()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));

/* XXX TODO: partial save for organizer items as well as contacts!!!

    QList<QOrganizerItem> items(oim->items());
    QList<QOrganizerItem> originalItems(items);
    QCOMPARE(items.count(), 3);

    QOrganizerItemLocalId aId = items[0].localId();
    QOrganizerItemLocalId bId = items[1].localId();
    QOrganizerItemLocalId cId = items[2].localId();

    // Test 1: saving a contact with a changed detail masked out does nothing
    QOrganizerItemPriority priority(items[0].detail<QOrganizerItemPriority>());
    priority.setPriority(QOrganizerItemPriority::LowPriority);
    items[0].saveDetail(&priority);

    QOrganizerItemSaveRequest isr;
    isr.setManager(oim.data());
    isr.setItems(items);
    isr.setDefinitionMask(QStringList(QOrganizerItemEmailAddress::DefinitionName));
    qRegisterMetaType<QOrganizerItemSaveRequest*>("QOrganizerItemSaveRequest*");
    QThreadSignalSpy spy(&isr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(isr.start());
    QVERIFY((isr.isActive() && isr.state() == QOrganizerItemAbstractRequest::ActiveState) || isr.isFinished());
    QVERIFY(isr.waitForFinished());
    QVERIFY(isr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(isr.error(), QOrganizerItemManager::NoError);
    QVERIFY(isr.errorMap().isEmpty());
    items[0] = oim->item(aId);
    QCOMPARE(items[0].detail<QOrganizerItemPriority>().priority(),
            originalContacts[0].detail<QOrganizerItemPriority>().priority());

    // Test 2: saving a contact with a changed detail in the mask changes it
    QOrganizerItemEmailAddress email;
    email.setEmailAddress("me@example.com");
    items[1].saveDetail(&email);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList(QOrganizerItemEmailAddress::DefinitionName));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QCOMPARE(isr.error(), QOrganizerItemManager::NoError);
    QVERIFY(isr.errorMap().isEmpty());
    items[1] = oim->item(bId);
    QCOMPARE(items[1].detail<QOrganizerItemEmailAddress>().emailAddress(), QString("me@example.com"));

    // 3) Remove an email address and a phone number
    QCOMPARE(items[1].details<QOrganizerItemPriority>().count(), 1);
    QCOMPARE(items[1].details<QOrganizerItemEmailAddress>().count(), 1);
    QVERIFY(items[1].removeDetail(&email));
    phn = items[1].detail<QOrganizerItemPriority>();
    QVERIFY(items[1].removeDetail(&priority));
    QVERIFY(items[1].details<QOrganizerItemEmailAddress>().count() == 0);
    QVERIFY(items[1].details<QOrganizerItemPriority>().count() == 0);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList(QOrganizerItemEmailAddress::DefinitionName));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QCOMPARE(isr.error(), QOrganizerItemManager::NoError);
    QVERIFY(isr.errorMap().isEmpty());
    items[1] = oim->item(bId);
    QCOMPARE(items[1].details<QOrganizerItemEmailAddress>().count(), 0);
    QCOMPARE(items[1].details<QOrganizerItemPriority>().count(), 1);

    // 4 - New contact, no details in the mask
    QOrganizerItem newContact;
    newContact.saveDetail(&email);
    newContact.saveDetail(&priority);
    items.append(newContact);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList(QOrganizerItemOnlineAccount::DefinitionName));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QCOMPARE(isr.error(), QOrganizerItemManager::NoError);
    QVERIFY(isr.errorMap().isEmpty());
    items = isr.items();
    QCOMPARE(items.size()-1, 3);  // Just check that we are dealing with the contact at index 3
    QOrganizerItemLocalId dId = items[3].localId();
    items[3] = oim->item(dId);
    QVERIFY(items[3].details<QOrganizerItemEmailAddress>().count() == 0); // not saved
    QVERIFY(items[3].details<QOrganizerItemPriority>().count() == 0); // not saved

    // 5 - New contact, some details in the mask
    QVERIFY(newContact.localId() == 0);
    QVERIFY(newContact.details<QOrganizerItemEmailAddress>().count() == 1);
    QVERIFY(newContact.details<QOrganizerItemPriority>().count() == 1);
    items.append(newContact);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList(QOrganizerItemPriority::DefinitionName));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QCOMPARE(isr.error(), QOrganizerItemManager::NoError);
    QVERIFY(isr.errorMap().isEmpty());
    items = isr.items();
    QCOMPARE(items.size()-1, 4);  // Just check that we are dealing with the contact at index 4
    QOrganizerItemLocalId eId = items[4].localId();
    items[4] = oim->item(eId);
    QCOMPARE(items[4].details<QOrganizerItemEmailAddress>().count(), 0); // not saved
    QCOMPARE(items[4].details<QOrganizerItemPriority>().count(), 1); // saved

    // 6) Have a bad manager uri in the middle followed by a save error
    QOrganizerItemId id3(items[3].id());
    QOrganizerItemId badId(id3);
    badId.setManagerUri(QString());
    items[3].setId(badId);
    QOrganizerItemDetail badDetail("BadDetail");
    badDetail.setValue("BadField", "BadValue");
    items[4].saveDetail(&badDetail);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList("BadDetail"));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QVERIFY(isr.error() != QOrganizerItemManager::NoError);
    QMap<int, QOrganizerItemManager::Error> errorMap(isr.errorMap());
    QCOMPARE(errorMap.count(), 2);
    QCOMPARE(errorMap[3], QOrganizerItemManager::DoesNotExistError);
    QCOMPARE(errorMap[4], QOrganizerItemManager::InvalidDetailError);

    // 7) Have a non existing contact in the middle followed by a save error
    badId = id3;
    badId.setLocalId(987234); // something nonexistent (hopefully)
    items[3].setId(badId);
    isr.setItems(items);
    isr.setDefinitionMask(QStringList("BadDetail"));
    QVERIFY(isr.start());
    QVERIFY(isr.waitForFinished());
    QVERIFY(isr.error() != QOrganizerItemManager::NoError);
    errorMap = isr.errorMap();
    QCOMPARE(errorMap.count(), 2);
    QCOMPARE(errorMap[3], QOrganizerItemManager::DoesNotExistError);
    QCOMPARE(errorMap[4], QOrganizerItemManager::InvalidDetailError);

XXX TODO: partial save for organizer items as well as contacts!!! */
}

void tst_QOrganizerItemAsync::definitionFetch()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerItemDetailDefinitionFetchRequest dfr;
    QVERIFY(dfr.type() == QOrganizerItemAbstractRequest::DetailDefinitionFetchRequest);
    QVERIFY(dfr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeNote))); // ensure ctor sets contact type correctly.
    dfr.setItemType(QOrganizerItemType::TypeNote);
    QVERIFY(dfr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeNote)));

    // initial state - not started, no manager.
    QVERIFY(!dfr.isActive());
    QVERIFY(!dfr.isFinished());
    QVERIFY(!dfr.start());
    QVERIFY(!dfr.cancel());
    QVERIFY(!dfr.waitForFinished());

    // "all definitions" retrieval
    dfr.setManager(oim.data());
    QCOMPARE(dfr.manager(), oim.data());
    QVERIFY(!dfr.isActive());
    QVERIFY(!dfr.isFinished());
    QVERIFY(!dfr.cancel());
    QVERIFY(!dfr.waitForFinished());
    qRegisterMetaType<QOrganizerItemDetailDefinitionFetchRequest*>("QOrganizerItemDetailDefinitionFetchRequest*");
    QThreadSignalSpy spy(&dfr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    dfr.setDefinitionNames(QStringList());
    QVERIFY(!dfr.cancel()); // not started
    QVERIFY(dfr.start());

    QVERIFY((dfr.isActive() && dfr.state() == QOrganizerItemAbstractRequest::ActiveState) || dfr.isFinished());
    //QVERIFY(dfr.isFinished() || !dfr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(dfr.waitForFinished());
    QVERIFY(dfr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QMap<QString, QOrganizerItemDetailDefinition> defs = oim->detailDefinitions(QOrganizerItemType::TypeNote);
    QMap<QString, QOrganizerItemDetailDefinition> result = dfr.definitions();
    QCOMPARE(defs, result);

    // specific definition retrieval
    QStringList specific;
    specific << QOrganizerItemLocation::DefinitionName;
    dfr.setItemType(QOrganizerItemType::TypeEvent);
    QVERIFY(dfr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeEvent)));
    dfr.setDefinitionName(QOrganizerItemLocation::DefinitionName);
    QVERIFY(dfr.definitionNames() == specific);
    QVERIFY(!dfr.cancel()); // not started
    QVERIFY(dfr.start());

    QVERIFY((dfr.isActive() && dfr.state() == QOrganizerItemAbstractRequest::ActiveState) || dfr.isFinished());
    //QVERIFY(dfr.isFinished() || !dfr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(dfr.waitForFinished());
    QVERIFY(dfr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    defs.clear();
    defs.insert(QOrganizerItemLocation::DefinitionName, oim->detailDefinition(QOrganizerItemLocation::DefinitionName));
    result = dfr.definitions();
    QCOMPARE(defs, result);

    // cancelling
    dfr.setDefinitionNames(QStringList());

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!dfr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(dfr.start());
        if (!dfr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            dfr.waitForFinished();
            dfr.setDefinitionNames(QStringList());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(dfr.waitForFinished());
        QVERIFY(dfr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!dfr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(dfr.start());
        if (!dfr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            dfr.waitForFinished();
            dfr.setDefinitionNames(QStringList());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        dfr.waitForFinished();
        QVERIFY(dfr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        break;
    }

}

void tst_QOrganizerItemAsync::definitionRemove()
{
    QFETCH(QString, uri);

    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    if (!oim->hasFeature(QOrganizerItemManager::MutableDefinitions)) {
       QSKIP("This contact manager does not support mutable definitions, can't remove a definition!", SkipSingle);
    }
    QOrganizerItemDetailDefinitionRemoveRequest drr;
    QVERIFY(drr.type() == QOrganizerItemAbstractRequest::DetailDefinitionRemoveRequest);
    QVERIFY(drr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeNote))); // ensure ctor sets contact type correctly.
    drr.setItemType(QOrganizerItemType::TypeEvent);
    drr.setDefinitionNames(QStringList());
    QVERIFY(drr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeEvent)));

    // initial state - not started, no manager.
    QVERIFY(!drr.isActive());
    QVERIFY(!drr.isFinished());
    QVERIFY(!drr.start());
    QVERIFY(!drr.cancel());
    QVERIFY(!drr.waitForFinished());

    // specific group removal
    int originalCount = oim->detailDefinitions().keys().size();
    QStringList removeIds;
    removeIds << oim->detailDefinitions().keys().first();
    drr.setDefinitionName(oim->detailDefinitions().keys().first());
    drr.setManager(oim.data());
    QCOMPARE(drr.manager(), oim.data());
    QVERIFY(!drr.isActive());
    QVERIFY(!drr.isFinished());
    QVERIFY(!drr.cancel());
    QVERIFY(!drr.waitForFinished());
    qRegisterMetaType<QOrganizerItemDetailDefinitionRemoveRequest*>("QOrganizerItemDetailDefinitionRemoveRequest*");
    QThreadSignalSpy spy(&drr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(drr.definitionNames() == removeIds);
    QVERIFY(!drr.cancel()); // not started
    QVERIFY(drr.start());

    QVERIFY((drr.isActive() && drr.state() == QOrganizerItemAbstractRequest::ActiveState) || drr.isFinished());
    //QVERIFY(drr.isFinished() || !drr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(drr.waitForFinished());
    QVERIFY(drr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 1);
    oim->detailDefinition(removeIds.first()); // check that it has already been removed.
    QCOMPARE(oim->error(), QOrganizerItemManager::DoesNotExistError);

    // remove (asynchronously) a nonexistent group - should fail.
    drr.setDefinitionNames(removeIds);
    QVERIFY(!drr.cancel()); // not started
    QVERIFY(drr.start());

    QVERIFY((drr.isActive() && drr.state() == QOrganizerItemAbstractRequest::ActiveState) || drr.isFinished());
    //QVERIFY(drr.isFinished() || !drr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(drr.waitForFinished());
    QVERIFY(drr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 1); // hasn't changed
    QCOMPARE(drr.error(), QOrganizerItemManager::DoesNotExistError);

    // remove with list containing one valid and one invalid id.
    removeIds << oim->detailDefinitions().keys().first();
    drr.setDefinitionNames(removeIds);
    QVERIFY(!drr.cancel()); // not started
    QVERIFY(drr.start());

    QVERIFY((drr.isActive() && drr.state() == QOrganizerItemAbstractRequest::ActiveState) || drr.isFinished());
    //QVERIFY(drr.isFinished() || !drr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(drr.waitForFinished());
    QVERIFY(drr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished signals
    spy.clear();

    QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 2); // only one more has been removed
    QVERIFY(drr.errorMap().count() == 1);
    QVERIFY(drr.errorMap().keys().contains(0));
    QCOMPARE(drr.errorMap().value(0), QOrganizerItemManager::DoesNotExistError);

    // remove with empty list - nothing should happen.
    removeIds.clear();
    drr.setDefinitionNames(removeIds);
    QVERIFY(!drr.cancel()); // not started
    QVERIFY(drr.start());

    QVERIFY(drr.isActive() || drr.isFinished());
    //QVERIFY(drr.isFinished() || !drr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(drr.waitForFinished());

    QVERIFY(drr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 2); // hasn't changed
    QCOMPARE(drr.error(), QOrganizerItemManager::NoError);  // no error but no effect.

    // cancelling
    removeIds.clear();
    removeIds << oim->detailDefinitions().keys().first();
    drr.setDefinitionNames(removeIds);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!drr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(drr.start());
        if (!drr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            drr.waitForFinished();
            drr.setDefinitionNames(removeIds);

            QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 3); // finished
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            // XXX should be readded
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(drr.waitForFinished());
        QVERIFY(drr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 2); // hasn't changed
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!drr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(drr.start());
        if (!drr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            drr.waitForFinished();
            drr.setDefinitionNames(removeIds);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        drr.waitForFinished();
        QVERIFY(drr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        QCOMPARE(oim->detailDefinitions().keys().size(), originalCount - 3); // hasn't changed
        break;
    }

}

void tst_QOrganizerItemAsync::definitionSave()
{
    QFETCH(QString, uri);

    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));

    if (!oim->hasFeature(QOrganizerItemManager::MutableDefinitions)) {

       QSKIP("This contact manager does not support mutable definitions, can't save a definition!", SkipSingle);
    }
    
    QOrganizerItemDetailDefinitionSaveRequest dsr;
    QVERIFY(dsr.type() == QOrganizerItemAbstractRequest::DetailDefinitionSaveRequest);
    QVERIFY(dsr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeNote))); // ensure ctor sets contact type correctly
    dsr.setItemType(QOrganizerItemType::TypeEvent);
    QVERIFY(dsr.itemType() == QString(QLatin1String(QOrganizerItemType::TypeEvent)));

    // initial state - not started, no manager.
    QVERIFY(!dsr.isActive());
    QVERIFY(!dsr.isFinished());
    QVERIFY(!dsr.start());
    QVERIFY(!dsr.cancel());
    QVERIFY(!dsr.waitForFinished());

    // save a new detail definition
    int originalCount = oim->detailDefinitions().keys().size();
    QOrganizerItemDetailDefinition testDef;
    testDef.setName("TestDefinitionId");
    QMap<QString, QOrganizerItemDetailFieldDefinition> fields;
    QOrganizerItemDetailFieldDefinition f;
    f.setDataType(QVariant::String);
    fields.insert("TestDefinitionField", f);
    testDef.setFields(fields);
    QList<QOrganizerItemDetailDefinition> saveList;
    saveList << testDef;
    dsr.setManager(oim.data());
    QCOMPARE(dsr.manager(), oim.data());
    QVERIFY(!dsr.isActive());
    QVERIFY(!dsr.isFinished());
    QVERIFY(!dsr.cancel());
    QVERIFY(!dsr.waitForFinished());
    qRegisterMetaType<QOrganizerItemDetailDefinitionSaveRequest*>("QOrganizerItemDetailDefinitionSaveRequest*");
    QThreadSignalSpy spy(&dsr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    dsr.setDefinition(testDef);
    QCOMPARE(dsr.definitions(), saveList);
    QVERIFY(!dsr.cancel()); // not started
    QVERIFY(dsr.start());

    QVERIFY((dsr.isActive() && dsr.state() == QOrganizerItemAbstractRequest::ActiveState) || dsr.isFinished());
    //QVERIFY(dsr.isFinished() || !dsr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(dsr.waitForFinished());
    QVERIFY(dsr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerItemDetailDefinition> expected;
    expected << oim->detailDefinition("TestDefinitionId");
    QList<QOrganizerItemDetailDefinition> result = dsr.definitions();
    QCOMPARE(expected, result);
    QVERIFY(expected.contains(testDef));
    QCOMPARE(oim->detailDefinitions().values().size(), originalCount + 1);

    // update a previously saved group
    fields.insert("TestDefinitionFieldTwo", f);
    testDef.setFields(fields);
    saveList.clear();
    saveList << testDef;
    dsr.setDefinitions(saveList);
    QCOMPARE(dsr.definitions(), saveList);
    QVERIFY(!dsr.cancel()); // not started
    QVERIFY(dsr.start());

    QVERIFY((dsr.isActive() && dsr.state() == QOrganizerItemAbstractRequest::ActiveState) || dsr.isFinished());
    //QVERIFY(dsr.isFinished() || !dsr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(dsr.waitForFinished());
    QVERIFY(dsr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    expected.clear();
    expected << oim->detailDefinition("TestDefinitionId");
    result = dsr.definitions();
    QCOMPARE(expected, result);
    QVERIFY(expected.contains(testDef));
    QCOMPARE(oim->detailDefinitions().values().size(), originalCount + 1);

    // cancelling
    fields.insert("TestDefinitionFieldThree - shouldn't get saved", f);
    testDef.setFields(fields);
    saveList.clear();
    saveList << testDef;
    dsr.setDefinitions(saveList);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!dsr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(dsr.start());
        if (!dsr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            dsr.waitForFinished();
            saveList.clear();
            saveList << testDef;
            dsr.setDefinitions(saveList);
            oim->removeDetailDefinition(testDef.name());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(dsr.waitForFinished());
        QVERIFY(dsr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        QList<QOrganizerItemDetailDefinition> allDefs = oim->detailDefinitions().values();
        QVERIFY(!allDefs.contains(testDef));
        QCOMPARE(oim->detailDefinitions().values().size(), originalCount + 1);

        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!dsr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(dsr.start());
        if (!dsr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            dsr.waitForFinished();
            saveList.clear();
            saveList << testDef;
            dsr.setDefinitions(saveList);
            oim->removeDetailDefinition(testDef.name());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        dsr.waitForFinished();
        QVERIFY(dsr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        QList<QOrganizerItemDetailDefinition> allDefs = oim->detailDefinitions().values();
        QVERIFY(!allDefs.contains(testDef));
        QCOMPARE(oim->detailDefinitions().values().size(), originalCount + 1);

        break;
    }

}

void tst_QOrganizerItemAsync::collectionFetch()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));

    QOrganizerCollectionFetchRequest cfr;
    QVERIFY(cfr.type() == QOrganizerItemAbstractRequest::CollectionFetchRequest);

    // initial state - not started, no manager.
    QVERIFY(!cfr.isActive());
    QVERIFY(!cfr.isFinished());
    QVERIFY(!cfr.start());
    QVERIFY(!cfr.cancel());
    QVERIFY(!cfr.waitForFinished());

    // collection retrieval by id.
    QList<QOrganizerCollectionLocalId> cids;
    cids << oim->collectionIds();
    cfr.setManager(oim.data());
    QCOMPARE(cfr.manager(), oim.data());
    QVERIFY(!cfr.isActive());
    QVERIFY(!cfr.isFinished());
    QVERIFY(!cfr.cancel());
    QVERIFY(!cfr.waitForFinished());
    qRegisterMetaType<QOrganizerCollectionFetchRequest*>("QOrganizerCollectionFetchRequest*");
    QThreadSignalSpy spy(&cfr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    cfr.setCollectionIds(cids);
    QCOMPARE(cfr.collectionIds(), cids);
    QVERIFY(!cfr.cancel()); // not started

    QVERIFY(cfr.start());
    //QVERIFY(cfr.isFinished() || !cfr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY((cfr.isActive() && cfr.state() == QOrganizerItemAbstractRequest::ActiveState) || cfr.isFinished());
    QVERIFY(cfr.waitForFinished());
    QVERIFY(cfr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerCollection> syncCols = oim->collections(cids);
    QList<QOrganizerCollection> cols = cfr.collections();
    QCOMPARE(cols.size(), syncCols.size());
    for (int i = 0; i < cols.size(); i++) {
        QOrganizerCollection curr = cols.at(i);
        QVERIFY(syncCols.contains(curr));
    }
    QVERIFY(cfr.errorMap().isEmpty()); // no error should have occurred.

    // cancelling
    cfr.setCollectionIds(cids);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!cfr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(cfr.start());
        if (!cfr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            spy.clear();
            cfr.waitForFinished();
            cfr.setCollectionIds(cids);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(cfr.waitForFinished());
        QVERIFY(cfr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!cfr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(cfr.start());
        if (!cfr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            cfr.waitForFinished();
            cfr.setCollectionIds(cids);
            bailoutCount -= 1;
            spy.clear();
            if (!bailoutCount) {
                //qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }
        cfr.waitForFinished();
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        QVERIFY(!cfr.isActive());
        QVERIFY(cfr.state() == QOrganizerItemAbstractRequest::CanceledState);
        break;
    }

    // Now test deletion in the first slot called
    QOrganizerCollectionFetchRequest *cfr2 = new QOrganizerCollectionFetchRequest();
    QPointer<QObject> obj(cfr2);
    cfr2->setManager(oim.data());
    connect(cfr2, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)), this, SLOT(deleteRequest()));
    QVERIFY(cfr2->start());
    int i = 100;
    // at this point we can't even call wait for finished..
    while(obj && i > 0) {
        QTest::qWait(50); // force it to process events at least once.
        i--;
    }
    QVERIFY(obj == NULL);
}

void tst_QOrganizerItemAsync::collectionIdFetch()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerCollectionLocalIdFetchRequest cifr;
    QVERIFY(cifr.type() == QOrganizerItemAbstractRequest::CollectionLocalIdFetchRequest);

    // initial state - not started, no manager.
    QVERIFY(!cifr.isActive());
    QVERIFY(!cifr.isFinished());
    QVERIFY(!cifr.start());
    QVERIFY(!cifr.cancel());
    QVERIFY(!cifr.waitForFinished());

    // "all collection ids" retrieval
    cifr.setManager(oim.data());
    QCOMPARE(cifr.manager(), oim.data());
    QVERIFY(!cifr.isActive());
    QVERIFY(!cifr.isFinished());
    QVERIFY(!cifr.cancel());
    QVERIFY(!cifr.waitForFinished());
    qRegisterMetaType<QOrganizerCollectionLocalIdFetchRequest*>("QOrganizerCollectionLocalIdFetchRequest*");

    QThreadSignalSpy spy(&cifr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(!cifr.cancel()); // not started
    QVERIFY(cifr.start());

    QVERIFY((cifr.isActive() && cifr.state() == QOrganizerItemAbstractRequest::ActiveState) || cifr.isFinished());
    //QVERIFY(cifr.isFinished() || !cifr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(cifr.waitForFinished());
    QVERIFY(cifr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerCollectionLocalId> colIds = oim->collectionIds();
    QList<QOrganizerCollectionLocalId> result = cifr.collectionIds();
    QCOMPARE(colIds, result);

    // cancelling
    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!cifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(cifr.start());
        if (!cifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            cifr.waitForFinished();
            bailoutCount -= 1;
            spy.clear();
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(cifr.waitForFinished());
        QVERIFY(cifr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!cifr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(cifr.start());
        if (!cifr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            cifr.waitForFinished();
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            continue;
        }
        cifr.waitForFinished();
        QVERIFY(cifr.isCanceled());

        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }
}

void tst_QOrganizerItemAsync::collectionRemove()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerCollectionRemoveRequest crr;
    QVERIFY(crr.type() == QOrganizerItemAbstractRequest::CollectionRemoveRequest);

    // initial state - not started, no manager.
    QVERIFY(!crr.isActive());
    QVERIFY(!crr.isFinished());
    QVERIFY(!crr.start());
    QVERIFY(!crr.cancel());
    QVERIFY(!crr.waitForFinished());

    // specific collection set
    QOrganizerCollectionLocalId removeId = oim->collectionIds().last();
    crr.setCollectionId(removeId);
    QVERIFY(crr.collectionIds() == QList<QOrganizerCollectionLocalId>() << removeId);
    int originalCount = oim->collectionIds().size();
    crr.setManager(oim.data());
    QCOMPARE(crr.manager(), oim.data());
    QVERIFY(!crr.isActive());
    QVERIFY(!crr.isFinished());
    QVERIFY(!crr.cancel());
    QVERIFY(!crr.waitForFinished());
    qRegisterMetaType<QOrganizerCollectionRemoveRequest*>("QOrganizerCollectionRemoveRequest*");
    QThreadSignalSpy spy(&crr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    QVERIFY(!crr.cancel()); // not started
    QVERIFY(crr.start());
    QVERIFY((crr.isActive() &&crr.state() == QOrganizerItemAbstractRequest::ActiveState) || crr.isFinished());
    //QVERIFY(crr.isFinished() || !crr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(crr.waitForFinished());
    QVERIFY(crr.isFinished());

    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QCOMPARE(oim->collectionIds().size(), originalCount - 1); // should have removed that particular collection.

    // remove all collections
    crr.setCollectionIds(oim->collectionIds());

    QVERIFY(!crr.cancel()); // not started
    QVERIFY(crr.start());

    QVERIFY((crr.isActive() && crr.state() == QOrganizerItemAbstractRequest::ActiveState) || crr.isFinished());
    //QVERIFY(crr.isFinished() || !crr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(crr.waitForFinished());
    QVERIFY(crr.isFinished());

    QVERIFY(oim->collectionIds().size() >= 1); // at least one collection must be left, since default collection cannot be removed.
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    // cancelling
    QOrganizerCollection temp;
    temp.setMetaData("description", "Should not be removed!");
    oim->saveCollection(&temp);
    crr.setCollectionId(temp.id().localId());

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!crr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(spy.count() == 0);
        QVERIFY(crr.start());
        if (!crr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            crr.waitForFinished();
            temp.setId(QOrganizerCollectionId());
            if (!oim->saveCollection(&temp)) {
                QSKIP("Unable to save temporary contact for remove request cancellation test!", SkipSingle);
            }
            crr.setCollectionId(temp.id().localId());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(crr.waitForFinished());
        QVERIFY(crr.isCanceled());
        QCOMPARE(oim->collectionIds().size(), 1);
        QCOMPARE(oim->collectionIds(), crr.collectionIds());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // restart, and wait for progress after cancel.
    while (true) {
        QVERIFY(!crr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(crr.start());
        if (!crr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            crr.waitForFinished();
            temp.setId(QOrganizerCollectionId());
            if (!oim->saveCollection(&temp)) {
                QSKIP("Unable to save temporary contact for remove request cancellation test!", SkipSingle);
            }
            crr.setCollectionId(temp.id().localId());
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        crr.waitForFinished();
        QVERIFY(crr.isCanceled());
        QCOMPARE(oim->collectionIds().size(), 1);
        QCOMPARE(oim->collectionIds(), crr.collectionIds());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();
        break;
    }

    // now clean up our temp collection.
    oim->removeCollection(temp.localId());
}

void tst_QOrganizerItemAsync::collectionSave()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    QOrganizerCollectionSaveRequest csr;
    QVERIFY(csr.type() == QOrganizerItemAbstractRequest::CollectionSaveRequest);

    // initial state - not started, no manager.
    QVERIFY(!csr.isActive());
    QVERIFY(!csr.isFinished());
    QVERIFY(!csr.start());
    QVERIFY(!csr.cancel());
    QVERIFY(!csr.waitForFinished());

    // save a new contact
    int originalCount = oim->collectionIds().size();
    QOrganizerCollection testCollection;
    testCollection.setMetaData("description", "test description");
    QList<QOrganizerCollection> saveList;
    saveList << testCollection;
    csr.setManager(oim.data());
    QCOMPARE(csr.manager(), oim.data());
    QVERIFY(!csr.isActive());
    QVERIFY(!csr.isFinished());
    QVERIFY(!csr.cancel());
    QVERIFY(!csr.waitForFinished());
    qRegisterMetaType<QOrganizerCollectionSaveRequest*>("QOrganizerCollectionSaveRequest*");
    QThreadSignalSpy spy(&csr, SIGNAL(stateChanged(QOrganizerItemAbstractRequest::State)));
    csr.setCollection(testCollection);
    QCOMPARE(csr.collections(), saveList);
    QVERIFY(!csr.cancel()); // not started
    QVERIFY(csr.start());

    QVERIFY((csr.isActive() && csr.state() == QOrganizerItemAbstractRequest::ActiveState) || csr.isFinished());
    //QVERIFY(csr.isFinished() || !csr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(csr.waitForFinished());
    QVERIFY(csr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    QList<QOrganizerCollection> expected = csr.collections();
    QCOMPARE(expected.size(), 1);
    QList<QOrganizerCollection> result = oim->collections(QList<QOrganizerCollectionLocalId>() << csr.collections().at(0).localId());
    // find the saved one, compare.
    foreach (const QOrganizerCollection& col, result) {
        if (col.id() == expected.at(0).id()) {
            QVERIFY(col == expected.at(0)); // XXX TODO: if we change the semantic so that save merely updates the id...?
        }
    }

    // update a previously saved collection
    testCollection = result.first();
    testCollection.setMetaData("name", "test name");
    saveList.clear();
    saveList << testCollection;
    csr.setCollections(saveList);
    QCOMPARE(csr.collections(), saveList);
    QVERIFY(!csr.cancel()); // not started
    QVERIFY(csr.start());

    QVERIFY((csr.isActive() && csr.state() == QOrganizerItemAbstractRequest::ActiveState) || csr.isFinished());
    //QVERIFY(csr.isFinished() || !csr.start());  // already started. // thread scheduling means this is untestable
    QVERIFY(csr.waitForFinished());

    QVERIFY(csr.isFinished());
    QVERIFY(spy.count() >= 1); // active + finished progress signals
    spy.clear();

    expected = csr.collections();
    result.clear();
    result = oim->collections();
    // find the saved one, compare.
    foreach (const QOrganizerCollection& col, result) {
        if (col.id() == expected.at(0).id()) {
            QVERIFY(col == expected.at(0)); // XXX TODO: if we change the semantic so that save merely updates the id...?
        }
    }
    QCOMPARE(oim->collectionIds().size(), originalCount + 1); // ie shouldn't have added an extra one (would be +2)

    // cancelling
    QOrganizerCollection temp = testCollection;
    temp.setMetaData("test", "shouldn't be saved");
    saveList.clear();
    saveList << temp;
    csr.setCollections(saveList);

    int bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT; // attempt to cancel 40 times.  If it doesn't work due to threading, bail out.
    while (true) {
        QVERIFY(!csr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(csr.start());
        if (!csr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            csr.waitForFinished();
            saveList = csr.collections();
            if (oim->collectionIds().size() > (originalCount + 1) && !oim->removeCollection(saveList.at(0).localId())) {
                QSKIP("Unable to remove saved collection to test cancellation of collection save request", SkipSingle);
            }
            saveList.clear();
            saveList << temp;
            csr.setCollections(saveList);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }

        // if we get here, then we are cancelling the request.
        QVERIFY(csr.waitForFinished());
        QVERIFY(csr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        expected.clear();
        QList<QOrganizerCollection> allCollections = oim->collections();
        QVERIFY(!allCollections.contains(temp)); // should NOT contain it since it was cancelled.
        QCOMPARE(oim->itemIds().size(), originalCount + 1);
        break;
    }
    // restart, and wait for progress after cancel.

    while (true) {
        QVERIFY(!csr.cancel()); // not started
        FILL_QUEUE_WITH_FETCH_REQUESTS();
        QVERIFY(csr.start());
        if (!csr.cancel()) {
            // due to thread scheduling, async cancel might be attempted
            // after the request has already finished.. so loop and try again.
            csr.waitForFinished();
            saveList = csr.collections();
            if (oim->collectionIds().size() > (originalCount + 1) && !oim->removeCollection(saveList.at(0).localId())) {
                QSKIP("Unable to remove saved contact to test cancellation of contact save request", SkipSingle);
            }
            saveList.clear();
            saveList << temp;
            csr.setCollections(saveList);
            bailoutCount -= 1;
            if (!bailoutCount) {
//                qWarning("Unable to test cancelling due to thread scheduling!");
                bailoutCount = MAX_OPTIMISTIC_SCHEDULING_LIMIT;
                break;
            }
            spy.clear();
            continue;
        }
        csr.waitForFinished(); // now wait until finished (if it hasn't already).
        QVERIFY(csr.isCanceled());
        QVERIFY(spy.count() >= 1); // active + cancelled progress signals
        spy.clear();

        // verify that the changes were not saved
        expected.clear();
        QList<QOrganizerCollection> allCollections = oim->collections();
        QVERIFY(!allCollections.contains(temp));
        QCOMPARE(oim->itemIds().size(), originalCount + 1);
        break;
    }
}


void tst_QOrganizerItemAsync::testQuickDestruction()
{
    QFETCH(QString, uri);

    // in this test, we create a manager, fire off a request, and delete the manager, all in quick succession
    // this is to test for segfaults etc.
    for (int i = 0; i < 10; i++) {
        QOrganizerItemFetchRequest ifr;
        QOrganizerItemManager *cm = prepareModel(uri);
        ifr.setManager(cm);
        ifr.start();
        delete cm;
    }
    // in this test, we create a manager, fire off a request, delete the request, then delete the manager, all in quick succession
    // this is to test for segfaults, etc.
    for (int i = 0; i < 10; i++) {
        QOrganizerItemFetchRequest *ifr = new QOrganizerItemFetchRequest;
        QOrganizerItemManager *cm = prepareModel(uri);
        ifr->setManager(cm);
        ifr->start();
        delete ifr;
        delete cm;
    }
    // in this test, we create a manager, fire off a request, delete the manager, then delete the request, all in quick succession
    // this is to test for segfaults, etc.
    for (int i = 0; i < 10; i++) {
        QOrganizerItemFetchRequest *ifr = new QOrganizerItemFetchRequest;
        QOrganizerItemManager *cm = prepareModel(uri);
        ifr->setManager(cm);
        ifr->start();
        delete cm;
        delete ifr;
    }
    // in this test, we create a manager, fire off a request, and delete the request, all in quick succession
    // this is to test for segfaults, etc.
    QOrganizerItemManager *cm = prepareModel(uri);
    for (int i = 0; i < 10; i++) {
        QOrganizerItemFetchRequest *ifr = new QOrganizerItemFetchRequest;
        ifr->setManager(cm);
        ifr->start();
        delete ifr;
    }
    delete cm;
}

void tst_QOrganizerItemAsync::threadDelivery()
{
    QFETCH(QString, uri);
    QScopedPointer<QOrganizerItemManager> oim(prepareModel(uri));
    m_mainThreadId = oim->thread()->currentThreadId();
    m_resultsAvailableSlotThreadId = m_mainThreadId;

    // now perform a fetch request and check that the progress is delivered to the correct thread.
    QOrganizerItemFetchRequest *req = new QOrganizerItemFetchRequest;
    req->setManager(oim.data());
    connect(req, SIGNAL(resultsAvailable()), this, SLOT(resultsAvailableReceived()));
    req->start();

    int totalWaitTime = 0;
    QTest::qWait(1); // force it to process events at least once.
    while (req->state() != QOrganizerItemAbstractRequest::FinishedState) {
        // ensure that the progress signal was delivered to the main thread.
        QCOMPARE(m_mainThreadId, m_resultsAvailableSlotThreadId);

        QTest::qWait(5); // spin until done
        totalWaitTime += 5;

        // break after 30 seconds.
        if (totalWaitTime > 30000) {
            delete req;
            QSKIP("Asynchronous request not complete after 30 seconds!", SkipSingle);
        }
    }

    // ensure that the progress signal was delivered to the main thread.
    QCOMPARE(m_mainThreadId, m_resultsAvailableSlotThreadId);
    delete req;
}

void tst_QOrganizerItemAsync::resultsAvailableReceived()
{
    QOrganizerItemFetchRequest *req = qobject_cast<QOrganizerItemFetchRequest *>(QObject::sender());
    if (req)
        m_resultsAvailableSlotThreadId = req->thread()->currentThreadId();
    else
        qWarning() << "resultsAvailableReceived() : request deleted; unable to set thread id!";
}

void tst_QOrganizerItemAsync::addManagers(QStringList stringlist)
{
    QTest::addColumn<QString>("uri");

    // retrieve the list of available managers
    QStringList managers = QOrganizerItemManager::availableManagers();

    // remove ones that we know will not pass
    if (!stringlist.contains("invalid"))
        managers.removeAll("invalid");
    if (!stringlist.contains("maliciousplugin"))
        managers.removeAll("maliciousplugin");
    if (!stringlist.contains("testdummy"))
        managers.removeAll("testdummy");
    if (!stringlist.contains("skeleton"))
        managers.removeAll("skeleton");

    foreach(QString mgr, managers) {
        QMap<QString, QString> params;
        QTest::newRow(QString("mgr='%1'").arg(mgr).toLatin1().constData()) << QOrganizerItemManager::buildUri(mgr, params);
        if (mgr == "memory") {
            params.insert("id", "tst_QOrganizerItemManager");
            QTest::newRow(QString("mgr='%1', params").arg(mgr).toLatin1().constData()) << QOrganizerItemManager::buildUri(mgr, params);
        }
    }
}

QOrganizerItemManager* tst_QOrganizerItemAsync::prepareModel(const QString& managerUri)
{
    QOrganizerItemManager* oim = QOrganizerItemManager::fromUri(managerUri);

    // XXX TODO: ensure that this is the case:
    // there should be no contacts in the database.
    QList<QOrganizerItemLocalId> toRemove = oim->itemIds();
    foreach (const QOrganizerItemLocalId& removeId, toRemove)
        oim->removeItem(removeId);

    QOrganizerItem a, b, c;
    QOrganizerItemDescription aDescriptionDetail;
    aDescriptionDetail.setDescription("A Description");
    a.saveDetail(&aDescriptionDetail);
    QOrganizerItemDescription bDescriptionDetail;
    bDescriptionDetail.setDescription("B Description");
    b.saveDetail(&bDescriptionDetail);
    QOrganizerItemDescription cDescriptionDetail;
    cDescriptionDetail.setDescription("C Description");
    c.saveDetail(&cDescriptionDetail);

    QOrganizerItemPriority priority;
    priority.setPriority(QOrganizerItemPriority::HighestPriority);
    c.saveDetail(&priority);
    priority.setPriority(QOrganizerItemPriority::VeryHighPriority);
    b.saveDetail(&priority);
    priority.setPriority(QOrganizerItemPriority::HighPriority);
    a.saveDetail(&priority);

    QOrganizerItemLocation loc;
    loc.setLocationName("http://test.nokia.com");
    a.saveDetail(&loc);

    oim->saveItem(&a);
    oim->saveItem(&b);
    oim->saveItem(&c);

    QOrganizerCollection testCollection;
    testCollection.setMetaData("testCollection", "test collection");
    oim->saveCollection(&testCollection);

    return oim;

    // TODO: cleanup once test is complete
}

QTEST_MAIN(tst_QOrganizerItemAsync)
#include "tst_qorganizeritemasync.moc"