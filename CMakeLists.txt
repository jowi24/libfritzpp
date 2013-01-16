# --- general setup -----------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
project (libfritz++)

#set(CMAKE_VERBOSE_MAKEFILE true) # <-- enable for debugging
#set(CMAKE_BUILD_TYPE "Debug")    # <-- enable for debugging 
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${libfritz++_SOURCE_DIR}/CMakeModules")
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# --- libgcrypt includes ------------------------------------------------------
include("FindGcryptConfig")
gcrypt_check(GCRYPT REQUIRED gcrypt)

# --- boost -------------------------------------------------------------------
find_package(Boost COMPONENTS system date_time thread regex REQUIRED)

# --- threading ---------------------------------------------------------------
find_package(Threads)

# --- crypto ------------------------------------------------------------------
find_package(crypto)

# --- gnu common cpp library --------------------------------------------------
pkg_check_modules(CC++ REQUIRED libccgnu2 libccext2)
include_directories(${CC++_INCLUDE_DIRS})
link_directories(${CC++_LIBRARY_DIRS})

# --- compile and link --------------------------------------------------------
include_directories(${libfritz++_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS} ${GCRYPT_CFLAGS} ${CC++_CFLAGS_OTHER}")

set(SRCS cc++/url.cpp cc++/urlstring.cpp cc++/mime.cpp cc++/soap.cpp
         CallList.cpp Config.cpp 
         Fonbooks.cpp Fonbook.cpp FonbookManager.cpp FritzClient.cpp FritzFonbook.cpp 
         HttpClient.cpp Listener.cpp LocalFonbook.cpp Log.cpp 
         LookupFonbook.cpp Nummerzoeker.cpp OertlichesFonbook.cpp 
         SoapClient.cpp TcpClient.cpp TelLocalChFonbook.cpp Tools.cpp XmlFonbook.cpp)
add_library(fritz++ STATIC ${SRCS})

# --- tests -------------------------------------------------------------------
if (EXISTS ${libfritz++_SOURCE_DIR}/test)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef -DSOURCE_DIR=\\\"${libfritz++_SOURCE_DIR}\\\"")
  include_directories(${libfritz++_SOURCE_DIR}/test)
  AUX_SOURCE_DIRECTORY(test LIBTESTFILES)
  add_executable(libfritztest ${LIBTESTFILES} test/gtest/gtest-all.cc test/gtest/gtest_main.cc)
  target_link_libraries(libfritztest fritz++  ssl
                        ${GCRYPT_LIBRARIES} ${CC++_LDFLAGS}
                        ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}
                        crypto)
endif (EXISTS ${libfritz++_SOURCE_DIR}/test)

