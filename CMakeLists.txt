cmake_minimum_required(VERSION 3.11)
project(LogView LANGUAGES CXX)

###
# Boost configuration
###
if(MSVC)
  message(STATUS "Toolset: MSVC")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(Boost_USE_STATIC_LIBS ON)
elseif(MINGW)
  message(STATUS "Toolset: gcc mingw")
  set(Boost_USE_STATIC_LIBS ON)
elseif(UNIX)
  message(STATUS "Toolset: gcc unix")
  set(Boost_USE_STATIC_LIBS OFF)
endif()
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS system filesystem iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "${Boost_LIBRARIES}")

###
# Qt configuration
###
if(WIN32)
  if(MSVC)
    message(STATUS "MSVC Version: " ${MSVC_VERSION})
    if(MSVC12)
      set(USED_QT_PATH "d:/Qt/5.10.1/msvc2013_64")
    elseif(${MSVC_VERSION} STREQUAL "1914" OR ${MSVC_VERSION} STREQUAL "1916")
      set(USED_QT_PATH "d:/Qt/5.10.1/msvc2017_64")
    else()
      message(FATAL_ERROR "Unsupported MSVC version")
    endif()
  elseif(MINGW)
    set(USED_QT_PATH "d:/Qt/5.10.1/mingw53_32")
  endif()
  set(ENV{CMAKE_PREFIX_PATH} "${USED_QT_PATH}/lib/cmake")
  message(STATUS "CMAKE_PREFIX_PATH: " $ENV{CMAKE_PREFIX_PATH})
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Gui Core Widgets REQUIRED)

set(SOURCES
  main.cpp
  logmodel.cpp
  logviewqt.cpp
  finddialog.cpp
)

set(HEADERS
  logmodel.h
  logviewqt.h
  finddialog.h
)

set(RESOURCES
  logviewqt.qrc
)

set(FORMS
  logviewqt.ui
  finddialog.ui
)

qt5_add_resources(RESOURCES_R ${RESOURCES})
qt5_wrap_ui(FORM_H ${FORMS})

###
# App configuration
###
set(DestinationDir ${LogView_SOURCE_DIR}/dist)
add_executable(LogView WIN32 ${SOURCES} ${HEADERS} ${FORM_H} ${RESOURCES_R})
target_link_libraries(LogView Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES})
install(TARGETS LogView RUNTIME DESTINATION ${DestinationDir})
