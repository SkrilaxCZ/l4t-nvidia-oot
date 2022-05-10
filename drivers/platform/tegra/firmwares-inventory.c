// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2022, NVIDIA CORPORATION, All rights reserved.

#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/tegra-firmwares.h>
#include <linux/cpu.h>
#include <asm/cpu.h>

#if IS_ENABLED(CONFIG_TRUSTY)
#include <linux/trusty/trusty.h>

static ssize_t tegrafw_read_trusty(struct device *dev,
				char *data, size_t size)
{
	const struct of_device_id trusty_of_match[] = {
		{.compatible = "android,trusty-smc-v1", },
		{},
	};
	struct device_node *dn;
	struct platform_device *pdev;

	for_each_matching_node(dn, trusty_of_match) {
		pdev = of_find_device_by_node(dn);
		if (pdev == NULL)
			continue;
		return snprintf(data, size, "%s",
			trusty_version_str_get(&pdev->dev));
	}
	snprintf(data, size, "NULL");
	return 0;
}
#endif

/* TBD: cpu_data is define in arch/arm64/kernel/cpuinfo.c
 * It is not exported and hence defining locally for the
 * build success.
 * Denver FW need to be deprecated.
 */
DEFINE_PER_CPU(struct cpuinfo_arm64, cpu_data);
static ssize_t tegrafw_read_denver(struct device *dev,
				char *version, size_t size)
{
	char *v = version;
	int i;
	size_t printed = 0;

	for_each_online_cpu(i) {
		struct cpuinfo_arm64 *cpuinfo = &per_cpu(cpu_data, i);
		u32 midr = cpuinfo->reg_midr;
		u32 aidr;

		if (MIDR_IMPLEMENTOR(midr) == ARM_CPU_IMP_NVIDIA) {
			asm volatile("mrs %0, AIDR_EL1" : "=r" (aidr) : );
			printed = snprintf(v, size, "CPU%d: %u(0x%x) ",
				i, aidr, aidr);
			size -= printed;
			v += printed;
		}
	}

	return v - version;
}

#define FIRMWARES_SIZE  10
static struct device *firmwares[FIRMWARES_SIZE];
#define check_out_of_bounds(dev, err) \
	if ((dev) - firmwares >= ARRAY_SIZE(firmwares)) { \
		pr_err("Cannot register 'legacy' firmware info: increase " \
			"firmwares array size"); \
		return (err); \
	}

static int __init tegra_firmwares_init(void)
{
	struct device **dev = firmwares;
	char *versions[] = { "mb1", "mb2", "mb1-bct", "qb", "osl" };
	int v;

	check_out_of_bounds(dev, 0);
	*dev++ = tegrafw_register("MTS", TFW_NORMAL, tegrafw_read_denver, NULL);

#if IS_ENABLED(CONFIG_TRUSTY)
	check_out_of_bounds(dev, 0);
	*dev++ = tegrafw_register("trusty", TFW_DONT_CACHE,
				tegrafw_read_trusty, NULL);
#endif

	for (v = 0; v < ARRAY_SIZE(versions); v++) {
		check_out_of_bounds(dev, 0);
		*dev++ = tegrafw_register_dt_string(versions[v],
			"/tegra-firmwares", versions[v]);
	}
	return 0;
}

static void __exit tegra_firmwares_exit(void)
{
	struct device **dev = firmwares;

	while (dev - firmwares < ARRAY_SIZE(firmwares))
		tegrafw_unregister(*dev++);
}
module_init(tegra_firmwares_init);
module_exit(tegra_firmwares_exit);

MODULE_DESCRIPTION("Firmware info drivers");
MODULE_AUTHOR("dmitry pervushin <dpervushin@nvidia.com>");
MODULE_AUTHOR("Laxman Dewangan <ldewangan@nvidia.com>");
MODULE_LICENSE("GPL v2");