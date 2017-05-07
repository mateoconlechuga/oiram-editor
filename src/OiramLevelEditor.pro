lessThan(QT_MAJOR_VERSION, 5) : error("You need at least Qt 5.6 to build!")
lessThan(QT_MINOR_VERSION, 6) : error("You need at least Qt 5.6 to build!")

QT       += core gui widgets

TARGET = OiramLevelEditor
TEMPLATE = app

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    #This is a debug build
    GLOBAL_FLAGS += -g3
}

# GCC/clang flags
if (!win32-msvc*) {
    GLOBAL_FLAGS    += -W -Wall -Wextra -Wunused-function -Werror=write-strings -Werror=redundant-decls -Werror=format -Werror=format-security -Werror=declaration-after-statement -Werror=implicit-function-declaration -Werror=date-time -Werror=missing-prototypes -Werror=return-type -Werror=pointer-arith -Winit-self
    GLOBAL_FLAGS    += -ffunction-sections -fdata-sections -fno-strict-overflow
    QMAKE_CFLAGS    += -std=gnu11
    QMAKE_CXXFLAGS  += -fno-exceptions
    isEmpty(CI) {
        # Only enable opts for non-CI release builds
        # -flto might cause an internal compiler error on GCC in some circumstances (with -g3?)... Comment it if needed.
        CONFIG(release, debug|release): GLOBAL_FLAGS += -O3 -flto
    }
} else {
    # TODO: add equivalent flags
    # Example for -Werror=shadow: /weC4456 /weC4457 /weC4458 /weC4459
    #     Source: https://connect.microsoft.com/VisualStudio/feedback/details/1355600/
    QMAKE_CXXFLAGS  += /Wall
}

if (macx|linux) {
    # Be more secure by default...
    GLOBAL_FLAGS    += -fPIE -Wstack-protector -fstack-protector-strong --param=ssp-buffer-size=1
    # Use ASAN on debug builds. Watch out about ODR crashes when built with -flto. detect_odr_violation=0 as an env var may help.
    CONFIG(debug, debug|release): GLOBAL_FLAGS += -fsanitize=address,bounds -fsanitize-undefined-trap-on-error -O0
}

macx:  QMAKE_LFLAGS += -Wl,-dead_strip
linux: QMAKE_LFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--gc-sections -pie

QMAKE_CFLAGS    += $$GLOBAL_FLAGS
QMAKE_CXXFLAGS  += $$GLOBAL_FLAGS
QMAKE_LFLAGS    += $$GLOBAL_FLAGS


SOURCES += main.cpp\
        mainwindow.cpp \
    tilemapview.cpp \
    tile.cpp \
    element.cpp \
    tileview.cpp \
    utils.cpp \
    level.c

HEADERS  += mainwindow.h \
    tilemapview.h \
    tile.h \
    element.h \
    tileview.h \
    utils.h \
    level.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
