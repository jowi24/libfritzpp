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

# --- compile and link --------------------------------------------------------
include_directories(${libfritz++_SOURCE_DIR})
include_directories(${libfritz++_SOURCE_DIR}/..)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCRYPT_CFLAGS} -std=gnu++11")

set(SRCS CallList.cpp Config.cpp 
         Fonbooks.cpp Fonbook.cpp FonbookManager.cpp FritzClient.cpp FritzFonbook.cpp 
         Listener.cpp LocalFonbook.cpp
         LookupFonbook.cpp Nummerzoeker.cpp OertlichesFonbook.cpp 
         TelLocalChFonbook.cpp Tools.cpp XmlFonbook.cpp)
add_library(fritz++ STATIC ${SRCS})

# --- tests -------------------------------------------------------------------
if (EXISTS ${libfritz++_SOURCE_DIR}/test)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef -DSOURCE_DIR=\\\"${libfritz++_SOURCE_DIR}\\\"")
  include_directories(${libfritz++_SOURCE_DIR}/test)
  AUX_SOURCE_DIRECTORY(test LIBTESTFILES)
  add_executable(libfritztest ${LIBTESTFILES} test/gtest/gtest-all.cc test/gtest/gtest_main.cc)
  target_link_libraries(libfritztest fritz++ log++ net++ conv++
                        ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY}
                        ${GCRYPT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
                        )
endif (EXISTS ${libfritz++_SOURCE_DIR}/test)

