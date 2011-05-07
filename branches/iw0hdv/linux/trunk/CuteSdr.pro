#-------------------------------------------------
#
# Project created by QtCreator 2010-09-15T13:53:54
# patched for U10.10, Andrea IW0HDV, Ken N9VV
#
#-------------------------------------------------

# check Qt version, 4.7is mandatory due to usage of some class introduced in Qt library only since release 4.7

QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)
lessThan(QT_VER_MAJ, 4) | lessThan(QT_VER_MIN, 7) {
   error(CuteSDR requires Qt 4.7 or newer but Qt $$[QT_VERSION] was detected.)
}


QT += core gui
QT += network

#
# Ubuntu 10.10 and 11.04: changes due to some strangeness in Debian Qt packages 
# 
linux-g++ {
    #
    # Try to catch distro informations
    #
    DISTRO=$$system(cat /etc/issue | head -n1 | cut -f1 -d\' \')
    DISTRO_MAJ=$$system(cat /etc/issue | head -n1 | cut -f2 -d\' \' | cut -f1 -d\\.)
    DISTRO_MIN=$$system(cat /etc/issue | head -n1 | cut -f2 -d\' \' | cut -f2 -d\\.)

    # specific to Ubuntu 10.10
    equals(DISTRO,Ubuntu):equals(DISTRO_MAJ,10):equals(DISTRO_MIN,10) {
       message(Linux $$DISTRO $$DISTRO_MAJ $$DISTRO_MIN)
       INCLUDEPATH += $$quote(/usr/include/QtMultimediaKit)
       LIBS += $$quote(-lQtMultimediaKit)
    } else {
       # specific to Ubuntu 11.04
       equals(DISTRO,Ubuntu):equals(DISTRO_MAJ,11):equals(DISTRO_MIN,04) {
          message(Linux $$DISTRO $$DISTRO_MAJ $$DISTRO_MIN)
          INCLUDEPATH += $$quote(/usr/include/QtMobility)
          INCLUDEPATH += $$quote(/usr/include/QtMultimediaKit)
          LIBS += $$quote(-lQtMultimediaKit)
       } else {
          QT += multimedia
       }
    }
}


linux-g++ {
  # gcc compiler optimization options, as per suggestion by Ken N9VV
  QMAKE_CXXFLAGS += -O3 \
                    -mfpmath=sse \
                    -msse \
                    -msse2 \
                    -msse3 \
                    -fomit-frame-pointer \
                    -ffast-math \
                    -march=i686 \
                    -fexceptions 
} 

TARGET = CuteSdr
TEMPLATE = app


SOURCES += gui/main.cpp \
    gui/sounddlg.cpp \
    gui/sdrsetupdlg.cpp \
    gui/sdrdiscoverdlg.cpp \
    gui/plotter.cpp \
    gui/mainwindow.cpp \
    gui/ipeditwidget.cpp \
    gui/freqctrl.cpp \
    gui/displaydlg.cpp \
    gui/demodsetupdlg.cpp \
	gui/editnetdlg.cpp \
	gui/testbench.cpp \
	gui/meter.cpp \
	gui/sliderctrl.cpp \
	gui/noiseprocdlg.cpp \
	gui/aboutdlg.cpp \
	interface/soundout.cpp \
    interface/sdrinterface.cpp \
    interface/netiobase.cpp \
    interface/ad6620.cpp \
	interface/perform.cpp \
	dsp/fractresampler.cpp \
    dsp/fastfir.cpp \
    dsp/downconvert.cpp \
    dsp/demodulator.cpp \
    dsp/fft.cpp \
	dsp/agc.cpp \
	dsp/amdemod.cpp \
	dsp/samdemod.cpp \
	dsp/ssbdemod.cpp \
	dsp/smeter.cpp \
    dsp/fmdemod.cpp \
	dsp/fir.cpp \
    dsp/iir.cpp \
	dsp/noiseproc.cpp


HEADERS  += gui/mainwindow.h \
	gui/sounddlg.h \
    gui/sdrsetupdlg.h \
    gui/sdrdiscoverdlg.h \
    gui/plotter.h \
	gui/ipeditwidget.h \
    gui/freqctrl.h \
	gui/sliderctrl.h \
	gui/editnetdlg.h \
    gui/displaydlg.h \
    gui/demodsetupdlg.h \
	gui/testbench.h \
	gui/meter.h \
	gui/noiseprocdlg.h \
	gui/aboutdlg.h \
	interface/soundout.h \
    interface/sdrinterface.h \
    interface/protocoldefs.h \
    interface/netiobase.h \
    interface/ad6620.h \
	interface/ascpmsg.h \
	interface/perform.h \
	dsp/fractresampler.h \
    dsp/fastfir.h \
	dsp/filtercoef.h \
	dsp/downconvert.h \
    dsp/demodulator.h \
    dsp/datatypes.h \
    dsp/fft.h \
	dsp/agc.h \
	dsp/amdemod.h \
	dsp/samdemod.h \
	dsp/ssbdemod.h \
	dsp/smeter.h \
    dsp/fmdemod.h \
	dsp/fir.h \
    dsp/iir.h \
	dsp/noiseproc.h

FORMS += gui/mainwindow.ui \
	gui/sdrdiscoverdlg.ui \
    gui/sounddlg.ui \
    gui/sdrsetupdlg.ui \
	gui/ipeditframe.ui \
    gui/editnetdlg.ui \
    gui/displaydlg.ui \
    gui/demodsetupdlg.ui \
    gui/testbench.ui \
    gui/sliderctrl.ui \
    gui/aboutdlg.ui \
    gui/noiseprocdlg.ui

unix:SOURCES +=
unix:!macx:SOURCES +=
macx {
	SOURCES +=
	LIBS += -framework \
		IOKit \
		-framework \
		CoreFoundation
}
win32 {
	SOURCES +=
	DEFINES += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
	LIBS += libwsock32
	RC_FILE = cutesdr.rc
}

linux-g++:DEFINES = _TTY_POSIX_ \
	_TTY_LINUX_
win32:DEFINES += _TTY_WIN_
win32:DEFINES += WINVER=0x0501
macx:DEFINES = _TTY_POSIX_ \
	_TTY_MACX_
CONFIG(debug, debug|release) {
	DESTDIR = debug/
	OBJECTS_DIR = debug/
	RCC_DIR = debug/
}
else {
	DESTDIR = release/
	OBJECTS_DIR = release/
	RCC_DIR = release/
}

OTHER_FILES += \
    cutesdr.rc
