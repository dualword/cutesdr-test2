project(siqs)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set (siqs_ftdi_SRCS
     siqs_ftdi.cpp
     discover.cpp
     logger.cpp
)

set (siqs_ftdi_INC
     netsupport.h
     minithread.h
     discover.h
     logger.h
)


find_library (rt_LIB           rt)
find_library (pthread_LIB      pthread)
find_library (usb-1.0_LIB      usb-1.0)
find_library (ftdi_LIB         ftdi)


ADD_DEFINITIONS ( -g -Wall -O3 -Wno-unused-function )

add_executable(siqs_ftdi ${siqs_ftdi_SRCS} ${siqs_ftdi_INC})

target_link_libraries(siqs_ftdi
                      ${rt_LIB}
                      ${pthread_LIB}
                      ${usb-1.0_LIB}
                      ${ftdi_LIB}
)

INSTALL_TARGETS(/bin siqs_ftdi)
