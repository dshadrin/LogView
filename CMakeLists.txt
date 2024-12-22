cmake_minimum_required(VERSION 3.15)
project(LogView LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

###
# Boost & other configuration
###
if(MSVC)
  message(STATUS "Toolset: MSVC")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(Boost_USE_STATIC_LIBS OFF)
elseif(MINGW)
  message(STATUS "Toolset: gcc mingw")
  set(Boost_USE_STATIC_LIBS ON)
  #set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
elseif(UNIX)
  message(STATUS "Toolset: gcc unix")
  set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_USE_MULTITHREADED ON)
set(Boost_NO_BOOST_CMAKE ON)

find_package(Boost 1.79 COMPONENTS system filesystem iostreams regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "${Boost_LIBRARIES}")

###
# Qt configuration
###
if(MSVC)
  message(STATUS "MSVC Version: " ${MSVC_VERSION})
  set(USED_QT_PATH "d:/Qt/6.2.2/msvc2019_64")
  set(QTN "Qt6")
  set(GUI_APP "WIN32")
elseif(MINGW)
  set(USED_QT_PATH "d:/Qt/6.2.2/mingw_64")
  set(QTN "Qt6")
  set(GUI_APP "WIN32")
else(UNIX)
  set(USED_QT_PATH $ENV{QTDIR})
  set(QTN "Qt6")
  set(GUI_APP "")
endif()
set(CMAKE_PREFIX_PATH "${USED_QT_PATH}/lib/cmake")
message(STATUS "CMAKE_PREFIX_PATH: " ${CMAKE_PREFIX_PATH})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(${QTN} COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
  main.cpp
  logmodel.cpp
  combomodel.cpp
  logviewqt.cpp
  finddialog.cpp
)

set(HEADERS
  logmodel.h
  combomodel.h
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

if(${QTN} STREQUAL "Qt6")
  qt6_add_resources(RESOURCES_R ${RESOURCES})
  qt6_wrap_ui(FORM_H ${FORMS})
else()
  qt5_add_resources(RESOURCES_R ${RESOURCES})
  qt5_wrap_ui(FORM_H ${FORMS})
endif()  

###
# App configuration
###
set(DestinationDir ${LogView_SOURCE_DIR}/dist)
add_executable(LogView ${GUI_APP} ${SOURCES} ${HEADERS} ${FORM_H} ${RESOURCES_R})
target_link_libraries(LogView ${QTN}::Gui ${QTN}::Widgets ${QTN}::Core ${Boost_LIBRARIES})
install(TARGETS LogView RUNTIME DESTINATION ${DestinationDir})
