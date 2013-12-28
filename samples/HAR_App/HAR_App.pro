#-------------------------------------------------
#
# Project created by QtCreator 2013-05-10T15:54:42
#
#-------------------------------------------------

QT       += core gui

TARGET = HAR_App
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cio.cpp \
    loaddatadlg.cpp \
    setparameterdlg.cpp

HEADERS  += mainwindow.h \
    cio.h \
    ../../include/configure.h \
    loaddatadlg.h \
    setparameterdlg.h \

FORMS    += mainwindow.ui \
    loaddatadlg.ui \
    setparameterdlg.ui
INCLUDEPATH += ../../include
INCLUDEPATH += /home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Include
# Include the configuration file below in the Qt .pro file, and modify the path accordingly.

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
# QtKinectWrapper   QtKinectWrapper   QtKinectWrapper   QtKinectWrapper
# ##############################################################################
# ##############################################################################
# Kinect: START OF CONFIGURATION BELOW ->
#
# Copy these lines at the end of your the project file of your application (.pro )
# Check the lines indicated "Modify below" and adapt the path to:
# 1) QtKinectWrapper sources (i.e. where the QKinectWrapper.* files lie),
# 2) OpenNI path
# 3) Make sure the OpenNI, PrimeSense NITE, PrimeSense Kinect Sensor are all in your path
#
# -------------------------------- Modify below --------------------------------

# Set QTKINECTWRAPPER_SOURCE_PATH to point to the directory containing the QtKinectWrapper classes
QTKINECTWRAPPER_SOURCE_PATH = ./QtKinectWrapper

# Set OPENNI_LIBRARY_PATH to point to the directory where openNI is installed. Assumption: the import libraries are in $$OPENNI_LIBRARY_PATH/lib
# Multiple path can be specified (e.g. to handle x86 / x64 differences)
# Quotation marks with spaces
win32 {
OPENNI_LIBRARY_PATH = "C:/Program Files (x86)/OpenNI"
OPENNI_LIBRARY_PATH += "C:/Program Files/OpenNI"
}

# ------------------------------ Copy as-is below ------------------------------
# ##############################################################################
# ##############################################################################
# QtKinectWrapper   QtKinectWrapper   QtKinectWrapper   QtKinectWrapper
# ##############################################################################
# Do not modify: default settings
# ##############################################################################
# Path for Qt wrapper
INCLUDEPATH += $$QTKINECTWRAPPER_SOURCE_PATH
# Sources for Qt wrapper
SOURCES += $$QTKINECTWRAPPER_SOURCE_PATH/QKinectWrapper.cpp
HEADERS += $$QTKINECTWRAPPER_SOURCE_PATH/QKinectWrapper.h



win32 {
   # Set the path to the patched openni/include
   INCLUDEPATH += $$QTKINECTWRAPPER_SOURCE_PATH/OpenNI/Include
   # Set the path to the Qt wrapper
   INCLUDEPATH += $$QTKINECTWRAPPER_SOURCE_PATH

   # Iterate through
   for(l, OPENNI_LIBRARY_PATH):LIBS+=-L$$l/lib
   #LIBS += -L$$OPENNI_LIBRARY_PATH/lib
   LIBS += -lopenNI
}

# ##############################################################################
# ##############################################################################
# QtKinectWrapper   QtKinectWrapper   QtKinectWrapper   QtKinectWrapper
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/release/ -lnimCodecs
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/debug/ -lnimCodecs
else:symbian: LIBS += -lnimCodecs
else:unix: LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/ -lnimCodecs

INCLUDEPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib
DEPENDPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/release/ -lnimMockNodes
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/debug/ -lnimMockNodes
else:symbian: LIBS += -lnimMockNodes
else:unix: LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/ -lnimMockNodes

INCLUDEPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib
DEPENDPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/release/ -lnimRecorder
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/debug/ -lnimRecorder
else:symbian: LIBS += -lnimRecorder
else:unix: LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/ -lnimRecorder

INCLUDEPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib
DEPENDPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/release/ -lOpenNI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/debug/ -lOpenNI
else:symbian: LIBS += -lOpenNI
else:unix: LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/ -lOpenNI

INCLUDEPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib
DEPENDPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/release/ -lOpenNI
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/debug/ -lOpenNI
else:symbian: LIBS += -lOpenNI
else:unix: LIBS += -L$$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib/ -lOpenNI

INCLUDEPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib
DEPENDPATH += $$PWD/../../../../../../../../../../home/allenchin/Kinect/OpenNI-Bin-Dev-Linux-x86-v1.5.4.0/Lib

unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lKNNClassifier

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libKNNClassifier.a

unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lSVMClassifier

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libSVMClassifier.a

unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lANN

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libANN.a

unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lSkeData

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libSkeData.a

unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lBaseClassifier

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libBaseClassifier.a



unix:!macx:!symbian: LIBS += -L$$PWD/../../lib/ -lsvm

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../../lib/libsvm.a


