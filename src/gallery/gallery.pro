TEMPLATE = lib

TARGET = QtGallery

include (../../common.pri)
INCLUDEPATH+= .

!static:DEFINES += QT_MAKEDLL
DEFINES += QT_BUILD_GALLERY_LIB

PRIVATE_HEADERS = \
        qabstractgallery_p.h \
        qgalleryabstractrequest_p.h \
        qgalleryabstractresponse_p.h \
        qgallerybaseresponse_p.h \
        qgalleryitemlist_p.h

PUBLIC_HEADERS = \
        qabstractgallery.h \
        qdocumentgallery.h \
        qgalleryabstractrequest.h \
        qgalleryabstractresponse.h \
        qgallerycountrequest.h \
        qgalleryfilter.h \
        qgalleryitemlist.h \
        qgalleryitemlistmodel.h \
        qgalleryitemrequest.h \
        qgalleryproperty.h \
        qgalleryqueryrequest.h \
        qgalleryremoverequest.h \
        qgallerytype.h \
        qgalleryurlrequest.h

SOURCES = \
        qabstractgallery.cpp \
        qdocumentgallery.cpp \
        qgalleryabstractrequest.cpp \
        qgalleryabstractresponse.cpp \
        qgallerycountrequest.cpp \
        qgallerybaseresponse.cpp \
        qgalleryfilter.cpp \
        qgalleryitemlist.cpp \
        qgalleryitemlistmodel.cpp \
        qgalleryitemrequest.cpp \
        qgalleryproperty.cpp \
        qgalleryqueryrequest.cpp \
        qgalleryremoverequest.cpp \
        qgallerytype.cpp \
        qgalleryurlrequest.cpp

HEADERS = $$PUBLIC_HEADERS $$PRIVATE_HEADERS

unix: contains(QT_CONFIG, dbus): include (maemo5/maemo5.pri)
else: DEFINES += QT_DOCUMENT_GALLERY_NULL

symbian {
    load(data_caging_paths)
    QtGalleryDeployment.sources = QtGallery.dll
    QtGalleryDeployment.path = /sys/bin
    DEPLOYMENT += QtGalleryDeployment
    TARGET.UID3=0x2002BFCB
    MMP_RULES += EXPORTUNFROZEN
    TARGET.CAPABILITY = ALL -TCB
}

CONFIG += middleware
include(../../features/deploy.pri)