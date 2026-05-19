cmake_minimum_required(VERSION 3.5)

project(OOP_project VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main.cpp

    # 核心逻辑
    src/core/tile.h
    src/core/tile.cpp
    src/core/dice.h
    src/core/dice.cpp
    src/core/effectcard.h
    src/core/effectcard.cpp
    src/core/questionbank.h
    src/core/questionbank.cpp
    src/core/player.h
    src/core/player.cpp
    src/core/board.h
    src/core/board.cpp
    src/core/game.h
    src/core/game.cpp
    src/core/config.h

    # 图形界面
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
    src/ui/boardwidget.h
    src/ui/boardwidget.cpp
    src/ui/playerpanel.h
    src/ui/playerpanel.cpp
    src/ui/dicewidget.h
    src/ui/dicewidget.cpp
    src/ui/eventlog.h
    src/ui/eventlog.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OOP_project
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(OOP_project SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(OOP_project
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(OOP_project PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.OOP_project)
endif()
set_target_properties(OOP_project PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS OOP_project
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OOP_project)
endif()
