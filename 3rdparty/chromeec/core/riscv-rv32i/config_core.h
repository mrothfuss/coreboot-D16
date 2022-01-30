/* Copyright 2019 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __CROS_EC_CONFIG_CORE_H
#define __CROS_EC_CONFIG_CORE_H

/* Linker binary architecture and format */
#define BFD_ARCH riscv
#define BFD_FORMAT "elf32-littleriscv"

#define CONFIG_SOFTWARE_CLZ
#define CONFIG_SOFTWARE_PANIC

#endif /* __CROS_EC_CONFIG_CORE_H */
