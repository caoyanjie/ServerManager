TEMPLATE = app

QT += qml quick
CONFIG += c++11

# x86 and x64 specific macros defines
CONFIG += debug_and_release
contains(QT_ARCH, i386) {
    DEFINES += ARCH_X86
    CONFIG(debug, debug|release){
        DESTDIR = $$PWD/../Bin/x86/Debug
        LIBS += -lRakNet_x86d
    }else{
        DESTDIR = $$PWD/../Bin/x86/Release
        LIBS += -lRakNet_x86
    }
}
else{
    DEFINES += ARCH_X64
    CONFIG(debug, debug|release){
        DESTDIR = $$PWD/../Bin/x64/Debug
        LIBS += -lRakNet_x64d
    }else{
        DESTDIR = $$PWD/../Bin/x64/Release
        LIBS += -lRakNet_x64
    }
}

# temp files dir
OBJECTS_DIR += $$PWD/../../Temp
RCC_DIR += $$PWD/../../Temp
MOC_DIR += $$PWD/../../Temp

# icon
RC_ICONS = Images/logo.ico

INCLUDEPATH += ../

LIBS += -L$$PWD/../StaticLib

LIBS += -lws2_32

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    DataProcess.h \
    DefineTypes.h \
    FileTransfer.h \
    ProcessJson.h \
    PubHeader.h \
    SystemPath.h \
    UdpTransmission.h \
    WriteLog.h \
    NATPunchthroghFacilitator.h

SOURCES += main.cpp \
    DataProcess.cpp \
    FileTransfer.cpp \
    ProcessJson.cpp \
    SystemPath.cpp \
    UdpTransmission.cpp \
    WriteLog.cpp \
    NATPunchthroghFacilitator.cpp

RESOURCES += qml.qrc

DISTFILES +=
