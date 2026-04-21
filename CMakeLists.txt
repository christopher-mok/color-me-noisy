cmake_minimum_required(VERSION 3.16)
project(colormenoisy VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(colormenoisy
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        image_utils.h image_utils.cpp
        cult.h cult.cpp
        patchmatch.h patchmatch.cpp
        image_pyramid.h image_pyramid.cpp
        settings.h settings.cpp
    )
else()
    if(ANDROID)
        add_library(colormenoisy SHARED ${PROJECT_SOURCES})
    else()
        add_executable(colormenoisy ${PROJECT_SOURCES})
    endif()
endif()

target_include_directories(colormenoisy PRIVATE "${CMAKE_SOURCE_DIR}/eigen")

# FFmpeg — prefer a local third_party/ffmpeg install, fall back to system
set(FFMPEG_LOCAL_DIR "${CMAKE_SOURCE_DIR}/third_party/ffmpeg")
if(EXISTS "${FFMPEG_LOCAL_DIR}/include/libavcodec/avcodec.h")
    message(STATUS "Using local FFmpeg from ${FFMPEG_LOCAL_DIR}")
    target_include_directories(colormenoisy PRIVATE "${FFMPEG_LOCAL_DIR}/include")
    target_link_directories(colormenoisy PRIVATE "${FFMPEG_LOCAL_DIR}/lib")
    target_link_libraries(colormenoisy PRIVATE avcodec avformat avutil swscale)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
    target_include_directories(colormenoisy PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_libraries(colormenoisy PRIVATE ${FFMPEG_LIBRARIES})
endif()

# Qt — must be linked explicitly
target_link_libraries(colormenoisy PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.colormenoisy)
endif()
set_target_properties(colormenoisy PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
include(GNUInstallDirs)
install(TARGETS colormenoisy
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(colormenoisy)
endif()
