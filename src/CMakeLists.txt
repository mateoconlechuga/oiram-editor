cmake_minimum_required(VERSION 3.21.1)

option(USE_QT5 "Build with Qt5 instead of Qt6" OFF)

if(USE_QT5)
    list(APPEND VCPKG_MANIFEST_FEATURES "qt5")
    if(APPLE)
        set(ENV{MACOSX_DEPLOYMENT_TARGET} "10.13")
        set(ENV{VCPKG_KEEP_ENV_VARS} "MACOSX_DEPLOYMENT_TARGET")
    endif()
else()
    list(APPEND VCPKG_MANIFEST_FEATURES "qt6")
    if(APPLE)
        set(ENV{MACOSX_DEPLOYMENT_TARGET} "11.0")
        set(ENV{VCPKG_KEEP_ENV_VARS} "MACOSX_DEPLOYMENT_TARGET")
    endif()
endif()

option(DEPS_RELEASE_ONLY "Build only release versions of vcpkg dependencies" OFF)

if(DEPS_RELEASE_ONLY)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        message(FATAL_ERROR "Must provide a VCPKG_TARGET_TRIPLET to set as release only")
    endif()
    if(NOT VCPKG_TARGET_TRIPLET MATCHES "-release$")
        set(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}-release")
        message("Updated VCPKG_TARGET_TRIPLET to ${VCPKG_TARGET_TRIPLET}")
    endif()
endif()

project(oiram-editor
        VERSION 2.0
        LANGUAGES C CXX)

if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "-static(-|$)" AND NOT VCPKG_TARGET_TRIPLET MATCHES "-md(-|$)")
    message("Using static MSVC runtime...")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SKIP_RPATH TRUE)

set(USED_CMAKE_GENERATOR "${CMAKE_GENERATOR}" CACHE STRING "Expose CMAKE_GENERATOR" FORCE)
message(STATUS "Detected system: ${CMAKE_SYSTEM_NAME} - host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR} - CXX_COMPILER: ${CMAKE_CXX_COMPILER_ID}")

# C11, and C++20 if supported otherwise C++14
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
include(CheckCXXCompilerFlag)
if(MSVC)
    CHECK_CXX_COMPILER_FLAG("/std:c++20" COMPILER_SUPPORTS_CXX20)
else()
    CHECK_CXX_COMPILER_FLAG("-std=c++2a" COMPILER_SUPPORTS_CXX20)
endif()
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20)
    add_definitions(-DTH_GDB_SUPPORT=1)
else()
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Sane flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /D_CRT_SECURE_NO_WARNINGS")
    if(MSVC_VERSION GREATER_EQUAL 1935 AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /experimental:c11atomics")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # sane defaults + hardening
    set(GLOBAL_COMPILE_FLAGS "-W -Wall -Wextra -Wno-unused-parameter -Werror=write-strings -Wredundant-decls -Werror=date-time -Werror=return-type -Werror=pointer-arith")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLOBAL_COMPILE_FLAGS} -Werror=implicit-function-declaration -Werror=missing-prototypes")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLOBAL_COMPILE_FLAGS}")
    # useful flags for debugging
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,bounds -fsanitize-undefined-trap-on-error ")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,bounds -fsanitize-undefined-trap-on-error ")
endif()

include(GNUInstallDirs)

if(USE_QT5)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
else()
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)
endif()

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
if(COMMAND qt_standard_project_setup)
    qt_standard_project_setup()
endif()

if(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)
endif()
if(WIN32)
    set(GUI_TYPE WIN32)
endif()

qt_add_resources(resources resources.qrc)

set(sources
    element.cpp element.h
    level.c level.h
    main.cpp
    mainwindow.cpp mainwindow.h
    tile.cpp tile.h
    tilemapview.cpp tilemapview.h
    tileview.cpp tileview.h
    utils.cpp utils.h
    ${resources}
)

if(WIN32)
    set(WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/os/windows/oiram-editor.rc")
endif()

if(USE_QT5)
    add_executable(oiram-editor ${GUI_TYPE} ${WINDOWS_ICON} ${sources})
else()
    qt_add_executable(oiram-editor ${GUI_TYPE}${WINDOWS_ICON} ${sources})
endif()

target_link_libraries(oiram-editor PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

execute_process(
    COMMAND git describe --abbrev=8 --dirty --always --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(oiram-editor PRIVATE
    VERSION_STRING=\"${VERSION_STRING}\"
)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set_target_properties(oiram-editor PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
    )
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()

target_compile_definitions(oiram-editor PRIVATE $<$<CONFIG:Release,RelWithDebInfo>:QT_NO_DEBUG_OUTPUT>)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/os/mac/icon.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(oiram-editor PUBLIC
        ${app_icon_macos}
    )
    set_target_properties(oiram-editor PROPERTIES
        MACOSX_BUNDLE_COPYRIGHT "mateoconlechuga"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_STRING}"
    )
endif()

install(TARGETS oiram-editor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

message("Binary dir: ${CMAKE_CURRENT_BINARY_DIR}")


if(COMMAND qt_generate_deploy_app_script)
    qt_generate_deploy_app_script(
        TARGET oiram-editor
        FILENAME_VARIABLE deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
