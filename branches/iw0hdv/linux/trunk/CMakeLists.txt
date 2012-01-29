project(cutesdr)
cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_VERBOSE_MAKEFILE OFF)

# Versions
set(cutesdr_VERSION_MAJOR 1)
set(cutesdr_VERSION_MINOR 0)
set(cutesdr_VERSION_PATCH 4)
set(cutesdr_VERSION ${cutesdr_VERSION_MAJOR}.${cutesdr_VERSION_MINOR}.${cutesdr_VERSION_PATCH})



# Source
add_subdirectory(siqs)

# Dependencies
find_package(PkgConfig REQUIRED)
find_package(Qt4 REQUIRED)

ADD_DEFINITIONS ( -D_REENTRANT -D_TTY_POSIX_ -D_TTY_LINUX_ -DQT_NO_DEBUG -DQT_MULTIMEDIA_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB )

set(CuteSDR_SRCS
    gui/main.cpp 
    gui/sounddlg.cpp 
    gui/sdrsetupdlg.cpp 
    gui/sdrdiscoverdlg.cpp 
    gui/plotter.cpp 
    gui/mainwindow.cpp 
    gui/ipeditwidget.cpp 
    gui/freqctrl.cpp 
    gui/displaydlg.cpp 
    gui/demodsetupdlg.cpp 
	gui/editnetdlg.cpp 
	gui/testbench.cpp 
	gui/meter.cpp 
	gui/sliderctrl.cpp 
	gui/noiseprocdlg.cpp 
	gui/aboutdlg.cpp 
    gui/rdsdecode.cpp
	interface/soundout.cpp 
    interface/sdrinterface.cpp 
    interface/netiobase.cpp 
    interface/ad6620.cpp 
	interface/perform.cpp 
	dsp/fractresampler.cpp 
    dsp/fastfir.cpp 
    dsp/downconvert.cpp 
    dsp/demodulator.cpp 
    dsp/fft.cpp 
	dsp/agc.cpp 
	dsp/amdemod.cpp 
	dsp/samdemod.cpp 
	dsp/ssbdemod.cpp 
	dsp/smeter.cpp 
    dsp/fmdemod.cpp 
	dsp/fir.cpp 
    dsp/iir.cpp 
	dsp/noiseproc.cpp
    dsp/wfmdemod.cpp
    dsp/wfmmod.cpp
)

set(CuteSDR_INC
    gui/mainwindow.h 
	gui/sounddlg.h 
    gui/sdrsetupdlg.h 
    gui/sdrdiscoverdlg.h 
    gui/plotter.h 
	gui/ipeditwidget.h 
    gui/freqctrl.h 
	gui/sliderctrl.h 
	gui/editnetdlg.h 
    gui/displaydlg.h 
    gui/demodsetupdlg.h 
	gui/testbench.h 
	gui/meter.h 
	gui/noiseprocdlg.h 
	gui/aboutdlg.h 
    gui/rdsdecode.h
	interface/soundout.h 
    interface/sdrinterface.h 
    interface/protocoldefs.h 
    interface/netiobase.h 
    interface/ad6620.h 
	interface/ascpmsg.h 
	interface/perform.h 
	dsp/fractresampler.h 
    dsp/fastfir.h 
	dsp/filtercoef.h 
	dsp/downconvert.h 
    dsp/demodulator.h 
    dsp/datatypes.h 
    dsp/fft.h 
	dsp/agc.h 
	dsp/amdemod.h 
	dsp/samdemod.h 
	dsp/ssbdemod.h 
	dsp/smeter.h 
    dsp/fmdemod.h 
	dsp/fir.h 
    dsp/iir.h 
	dsp/noiseproc.h
    dsp/wfmdemod.h 
    dsp/wfmmod.h
    dsp/rbdsconstants.h
)

set (CuteSDR_UIS
    gui/mainwindow.ui 
	gui/sdrdiscoverdlg.ui 
    gui/sounddlg.ui 
    gui/sdrsetupdlg.ui 
	gui/ipeditframe.ui 
    gui/editnetdlg.ui 
    gui/displaydlg.ui 
    gui/demodsetupdlg.ui 
    gui/testbench.ui 
    gui/sliderctrl.ui 
    gui/aboutdlg.ui 
    gui/noiseprocdlg.ui
)


#--------------------------------------------------------------
#
# Check for Ubuntu non standard installation
#
message ("qmake: ${QT_QMAKE_EXECUTABLE}")  

# get the path of the Qt header 
#
execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS
    OUTPUT_VARIABLE QT_INSTALL_HEADERS
    RESULT_VARIABLE rm_retval
)

# trim the last directory in path
string(REGEX REPLACE "(.*/)[^/]+$" "\\1" QT_INSTALL_HEADERS_UP ${QT_INSTALL_HEADERS})

# trim the end of line character
string (REGEX REPLACE "\n" ""  QT_INSTALL_HEADERS ${QT_INSTALL_HEADERS})

message("DIR INCLUDE: ${QT_INSTALL_HEADERS}")
message("DIR INSTALL HEADERS UP: ${QT_INSTALL_HEADERS_UP}")

# get the path of the Qt librarires 
#
execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_INSTALL_LIBS
    RESULT_VARIABLE rm_retval
)
# trim the end of line character
string (REGEX REPLACE "\n" ""  QT_INSTALL_LIBS ${QT_INSTALL_LIBS})



# default for standard QtSDK and binary packages before U11.04
#
set (QT_ADDITIONAL_INCLUDE_PATH "")
set (QT_ADDITIONAL_INCLUDE_PATH2 "")
set (QT_ADDITIONAL_LDFLAG "QtMultimedia" )


# if there is a suspicious QtMultimediaKit (sibling of the standard Qt include path)
# we have hit an Ubuntu binary package (11.04)
#
if ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )

    set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS_UP}/QtMultimediaKit/ )
    set (QT_ADDITIONAL_LDFLAG QtMultimediaKit)
    set (QT_ADDITIONAL_INCLUDE_PATH2 ${QT_INSTALL_HEADERS_UP}/QtMobility/ )

else ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )

    #set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS} )
    set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS}/QtMultimedia/ )
    set (QT_ADDITIONAL_LINK_DIR ${QT_INSTALL_LIBS})

endif ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )


message("QT_ADDITIONAL_INCLUDE_PATH:  ${QT_ADDITIONAL_INCLUDE_PATH}")
message("QT_ADDITIONAL_INCLUDE_PATH2: ${QT_ADDITIONAL_INCLUDE_PATH2}")
message("QT_ADDITIONAL_LDFLAG: ${QT_ADDITIONAL_LDFLAG}")

#--------------------------------------------------------------

#
# Build
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/dsp
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/interface

    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_MULTIMEDIA_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}

    #
    # non standard include path in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_INCLUDE_PATH}
    ${QT_ADDITIONAL_INCLUDE_PATH2}
)

link_directories(
    #
    # non standard library path in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_LINK_DIR}
)

QT4_WRAP_UI( CuteSDR_UI_HDRS ${CuteSDR_UIS} )


qt4_wrap_cpp (CuteSDR_MOCS ${CuteSDR_INC} )

add_executable(CuteSDR ${CuteSDR_SRCS} ${CuteSDR_UI_HDRS} ${CuteSDR_MOCS} )


target_link_libraries ( CuteSDR
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_MULTIMEDIA_LIBRARY}

    #
    # additional library in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_LDFLAG}
)



INSTALL_TARGETS(/bin CuteSDR)

#
# uninstall target
# workaround as found in http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
#
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


