# cmake/toolchains/Vita.cmake
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Pingus Vita Toolchain File
# Copyright (C) 2026 theheroGAC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED ENV{VITASDK})
  message(FATAL_ERROR "Please set VITASDK in your environment. "
                      "Example: export VITASDK=/usr/local/vitasdk")
endif()

set(VITASDK $ENV{VITASDK})

set(CMAKE_C_COMPILER   "${VITASDK}/bin/arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "${VITASDK}/bin/arm-vita-eabi-g++")
set(CMAKE_AR           "${VITASDK}/bin/arm-vita-eabi-ar"     CACHE FILEPATH "Archiver")
set(CMAKE_RANLIB       "${VITASDK}/bin/arm-vita-eabi-ranlib" CACHE FILEPATH "Ranlib")
set(CMAKE_STRIP        "${VITASDK}/bin/arm-vita-eabi-strip"  CACHE FILEPATH "Strip")

set(VITA ON CACHE BOOL "Build for PlayStation Vita" FORCE)

set(CMAKE_FIND_ROOT_PATH "${VITASDK}/arm-vita-eabi")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(VITA_ARCH_FLAGS "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${VITA_ARCH_FLAGS} -D__VITA__ -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_INIT "${VITA_ARCH_FLAGS} -D__VITA__ -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS_RELEASE   "-O3 -ftree-vectorize -ffast-math -DNDEBUG" CACHE STRING "C Release flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ftree-vectorize -ffast-math -DNDEBUG" CACHE STRING "CXX Release flags")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -g" CACHE STRING "C Debug flags")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g" CACHE STRING "CXX Debug flags")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-L${VITASDK}/arm-vita-eabi/lib -Wl,-q -Wl,-z,noexecstack -Wl,--no-warn-execstack")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
