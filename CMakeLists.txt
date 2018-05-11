cmake_minimum_required(VERSION 3.11)
project(LogView LANGUAGES CXX)

if(WIN32)
  if(MSVC)
    set(USED_QT_PATH "d:/Qt/Qt5.9.1/5.9.1/msvc2017_64")
  elseif(MINGW)
    set(USED_QT_PATH "d:/Qt/Qt5.9.1/5.10.1/mingw53_32")
  endif()
endif()

set(ENV{CMAKE_PREFIX_PATH} "${USED_QT_PATH}/lib/cmake")
message(STATUS "CMAKE_PREFIX_PATH: " $ENV{CMAKE_PREFIX_PATH})

set(CMAKE_CXX_STANDARD 14)

if(MSVC)
  message(STATUS "Toolset: MSVC")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(Boost_USE_STATIC_LIBS ON)
elseif(MINGW)
  message(STATUS "Toolset: gcc mingw")
  set(Boost_USE_STATIC_LIBS OFF)
elseif(UNIX)
  message(STATUS "Toolset: gcc unix")
  set(Boost_USE_STATIC_LIBS OFF)
endif()
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Gui Core Widgets)

set(SOURCES
  main.cpp
  logmodel.cpp
  logviewqt.cpp
)

set(HEADERS
  logmodel.h
  logviewqt.h
)

set(RESOURCES
  logviewqt.qrc
)

set(FORMS
  logviewqt.ui
)

qt5_add_resources(RESOURCES_R ${RESOURCES})
qt5_wrap_ui(FORM_H ${FORMS})
add_executable(LogView ${SOURCES} ${FORM_H} ${RESOURCES_R})
target_link_libraries(LogView Qt5::Core Qt5::Gui Qt5::Widgets ${Boost_LIBRARIES})
