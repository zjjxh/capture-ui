QT += quick
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    serialport.cpp \
    interaction_controller.cpp \
    gst_bus_poller.cpp \
    video-capture.cpp

RESOURCES += qml.qrc

QT+=serialbus
QT+=serialport
QT+= qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig
PKGCONFIG = \
    gstreamer-1.0

# conf for Magewell SDK
INCLUDEPATH += ../Magewell_Capture_SDK_Linux_3.3.1.813/Include
LIBS+= -L../Magewell_Capture_SDK_Linux_3.3.1.813/Lib/x64/ -lMWCapture -ludev -lv4l2 -lasound
QMAKE_CXXFLAGS += \
          -Wno-unused-parameter \
          -Wno-sign-compare \
          -Wno-unused-variable \
          -Wno-unused-but-set-variable \
          -Wno-missing-field-initializers

HEADERS += \
    serialport.h \
    interaction_controller.h \
    gst_bus_poller.h \
    video-capture.h \
    bitmap.h

DISTFILES += \
    popup.qml
