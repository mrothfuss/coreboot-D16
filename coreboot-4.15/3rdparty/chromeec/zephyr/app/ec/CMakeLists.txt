# Copyright 2020 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

zephyr_library_include_directories(include)
zephyr_library_sources(ec_app_main.c)
if(NOT DEFINED CONFIG_ZTEST)
    zephyr_library_sources(main_shim.c)
endif()
