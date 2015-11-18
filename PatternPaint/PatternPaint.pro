#-------------------------------------------------
#
# Pattern Paint
# http://blinkinlabs.com/patternpaint
#
# Copyright 2013, 2014, 2015 Blinkinlabs, LLC
#
#-------------------------------------------------

QT += widgets gui core

# Specificy where to place the object files so they don't clutter the source tree
OBJECTS_DIR = tmp
MOC_DIR = $$OBJECTS_DIR/moc
RCC_DIR = $$OBJECTS_DIR/rcc
UI_DIR = $$OBJECTS_DIR/uic

TARGET = PatternPaint
TEMPLATE = app

#Application version
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0

#Target version and application information
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

DEFINES += VERSION_STRING=\\\"$${VERSION}\\\"
DEFINES += APPLICATION_NAME=\\\"$${TARGET}\\\"
DEFINES += ORGANIZATION_NAME=\\\"Blinkinlabs\\\"
DEFINES += ORGANIZATION_DOMAIN=\\\"blinkinlabs.com\\\"

mac {
    QMAKE_TARGET_BUNDLE_PREFIX = com.blinkinlabs
}

# Disable updater checks when debugging, since the test app isn't signed and will fail anyway.
CONFIG(debug, debug|release) {
    DEFINES += DISABLE_UPDATE_CHECKS
}

# Workaround for lineinstrument tool in QT 5.2 - 5.4
# See: https://github.com/Blinkinlabs/PatternPaint/issues/66
equals(QT_MAJOR_VERSION, 5){
    greaterThan(QT_MINOR_VERSION,1) {
        lessThan(QT_MINOR_VERSION,5) {
            DEFINES += LINE_INSTRUMENT_WORKAROUND
        }
    }
}

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    systeminformation.cpp \
    aboutpatternpaint.cpp \
    addressprogrammer.cpp \
    letterboxscrollarea.cpp \
    colorchooser.cpp \
    patternframemodel.cpp \
    pattern.cpp \
    patterncollectionmodel.cpp \
    patterncollectionlistview.cpp \
    patterncollectiondelegate.cpp \
    patterncollection.cpp \
    frameeditor.cpp \
    patternwriter.cpp \
    patternscrollmodel.cpp \
    patterndelegate.cpp \
    patternframeundocommand.cpp \
    patternscrollundocommand.cpp \
    fixturesettings.cpp

HEADERS  += \
    mainwindow.h \
    systeminformation.h \
    aboutpatternpaint.h \
    addressprogrammer.h \
    letterboxscrollarea.h \
    colorchooser.h \
    patternframemodel.h \
    pattern.h \
    patterncollectionmodel.h \
    patterncollectionlistview.h \
    patterncollectiondelegate.h \
    patterncollection.h \
    frameeditor.h \
    patternwriter.h \
    patternscrollmodel.h \
    patternmodel.h \
    patterndelegate.h \
    colors.h \
    patternframeundocommand.h \
    patternscrollundocommand.h \
    fixturesettings.h


FORMS    += \
    mainwindow.ui \
    systeminformation.ui \
    aboutpatternpaint.ui \
    addressprogrammer.ui \
    fixturesettings.ui

RESOURCES += \
    images.qrc \
    examples.qrc

include(instruments/instruments.pri)
include(controllers/controllers.pri)
include(fixtures/fixtures.pri)
include(updater/updater.pri)


# Copies the given files to the destination directory
defineTest(copyToFrameworks) {
    files = $$1
    DDIR = PatternPaint.app/Contents/Frameworks

    QMAKE_POST_LINK += mkdir -p $$quote($$DDIR) $$escape_expand(\\n\\t)

    for(FILE, files) {
        QMAKE_POST_LINK += cp -R $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

macx {
    # OS X: Specify icon resource to use
    ICON = images/patternpaint.icns

    # OS X: Specify our developer information using a custom plist
    QMAKE_INFO_PLIST = Info.plist

    HEADERS  += \
        appnap.h

    OBJECTIVE_SOURCES += \
        appnap.mm

    # Workaround for broken macdeployqt on Qt 5.5.1: Copy in the system
    # libraries manually
    equals(QT_VERSION, 5.5.1){

        QTLIBS_PATH = ~/qt/5.5/clang_64/lib
#        TARGET_PATH = Contents/Frameworks

        SYSTEM_LIBS += \
            $$QTLIBS_PATH/QtCore.framework \
            $$QTLIBS_PATH/QtDBus.framework \
            $$QTLIBS_PATH/QtGui.framework \
            $$QTLIBS_PATH/QtPrintSupport.framework \
            $$QTLIBS_PATH/QtSerialPort.framework \
            $$QTLIBS_PATH/QtWidgets.framework

        copyToFrameworks($$SYSTEM_LIBS)

        # And add frameworks to the rpath so that the app can find the framework.
        QMAKE_RPATHDIR += @executable_path/../Frameworks
    }
}

win32 {
    # Windows: Specify the icon to use
    RC_ICONS += images/patternpaint.ico
}
