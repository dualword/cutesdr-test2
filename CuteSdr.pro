#-------------------------------------------------
#
# Project created by QtCreator 2010-09-15T13:53:54
#
#-------------------------------------------------
QT += core gui
QT += network
QT += multimedia
QT += widgets

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
	gui/rdsdecode.cpp \
	interface/soundout.cpp \
    interface/sdrinterface.cpp \
    interface/netiobase.cpp \
    interface/ad6620.cpp \
	interface/perform.cpp \
	interface/dataprocess.cpp \
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
	dsp/noiseproc.cpp \
    dsp/wfmdemod.cpp \
	dsp/wfmmod.cpp


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
	gui/rdsdecode.h \
	interface/soundout.h \
    interface/sdrinterface.h \
    interface/protocoldefs.h \
    interface/netiobase.h \
    interface/ad6620.h \
	interface/ascpmsg.h \
	interface/perform.h \
	dsp/fractresampler.h \
	interface/threadwrapper.h \
	interface/dataprocess.h \
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
	dsp/noiseproc.h \
    dsp/wfmdemod.h \
    dsp/wfmmod.h \
	dsp/rbdsconstants.h

#Use separate forms for each OS
win32 {
FORMS += winforms/mainwindow.ui \
	winforms/sdrdiscoverdlg.ui \
	winforms/sounddlg.ui \
	winforms/sdrsetupdlg.ui \
	winforms/ipeditframe.ui \
	winforms/editnetdlg.ui \
	winforms/displaydlg.ui \
	winforms/demodsetupdlg.ui \
	winforms/testbench.ui \
	winforms/sliderctrl.ui \
	winforms/aboutdlg.ui \
	winforms/noiseprocdlg.ui
}

macx {
FORMS += macforms/mainwindow.ui \
	macforms/sdrdiscoverdlg.ui \
	macforms/sounddlg.ui \
	macforms/sdrsetupdlg.ui \
	macforms/ipeditframe.ui \
	macforms/editnetdlg.ui \
	macforms/displaydlg.ui \
	macforms/demodsetupdlg.ui \
	macforms/testbench.ui \
	macforms/sliderctrl.ui \
	macforms/aboutdlg.ui \
	macforms/noiseprocdlg.ui
}
unix {
FORMS += unixforms/mainwindow.ui \
	unixforms/sdrdiscoverdlg.ui \
	unixforms/sounddlg.ui \
	unixforms/sdrsetupdlg.ui \
	unixforms/ipeditframe.ui \
	unixforms/editnetdlg.ui \
	unixforms/displaydlg.ui \
	unixforms/demodsetupdlg.ui \
	unixforms/testbench.ui \
	unixforms/sliderctrl.ui \
	unixforms/aboutdlg.ui \
	unixforms/noiseprocdlg.ui
}
unix:SOURCES +=
unix:!macx:SOURCES +=

macx {
	SOURCES +=
	ICON=cutesdr1.icns
}
win32 {
	SOURCES +=
	LIBS += -lws2_32
	RC_FILE = cutesdr.rc
}

OTHER_FILES += \
    cutesdr.rc
