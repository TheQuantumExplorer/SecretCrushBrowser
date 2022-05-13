## 0.0.1
cmake_minimum_required(VERSION 3.5)

project(SecretCrush VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
set(TS_FILES SecretCrush_en_US.ts)

set(PROJECT_SOURCES
        resources/resources.qrc
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        traductions/${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SecretCrush
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SecretCrush APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(WIN32)
        set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/icon.ico")
        add_executable(SecretCrush
            ${PROJECT_SOURCES}
            ${app_icon_resource_windows}
        )
    elseif(APPLE)
        set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/images/icon.icns")
        set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        set_target_properties(SecretCrush PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
            MACOSX_BUNDLE_ICON_FILE icon.icns
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )
        add_executable(SecretCrush
            ${PROJECT_SOURCES}
            ${app_icon_resource_windows}
            ${app_icon_macos}
        )
    else()
        add_executable(SecretCrush
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(SecretCrush PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(SecretCrush PRIVATE Qt6::WebEngineWidgets)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SecretCrush)
endif()
