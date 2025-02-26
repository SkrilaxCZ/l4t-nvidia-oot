/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023, NVIDIA CORPORATION & AFFILIATES.  All rights reserved.
 */

#ifndef _TEGRA_OOT_PRODS_H
#define _TEGRA_OOT_PRODS_H

#include <linux/version.h>

#ifdef CONFIG_TEGRA_PROD_ENABLE
#include <linux/tegra_prod.h>
#else
#include <linux/tegra-prod-legacy-dummy.h>
#endif

#endif /* _TEGRA_OOT_PRODS_H */
