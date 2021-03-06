/*
 * Copyright (c) 2016 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @defgroup sun4u sun4u platform driver.
 * @brief HelenOS sun4u platform driver.
 * @{
 */

/** @file
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <fibril_synch.h>
#include <stdlib.h>
#include <str.h>
#include <ctype.h>
#include <macros.h>

#include <ddi.h>
#include <ddf/driver.h>
#include <ddf/log.h>
#include <ipc/dev_iface.h>
#include <ops/hw_res.h>
#include <ops/pio_window.h>
#include <byteorder.h>

#define NAME "sun4u"

#define PBM_BASE		UINT64_C(0x1fe00000000)
#define PBM_SIZE		UINT64_C(0x00200000000)

#define PBM_PCI_CONFIG_BASE	UINT64_C(0x00001000000)
#define PBM_PCI_CONFIG_SIZE	UINT64_C(0x00001000000)

#define PBM_PCI_IO_BASE		UINT64_C(0x00002000000)
#define PBM_PCI_IO_SIZE		UINT64_C(0x00001000000)

#define PBM_PCI_MEM_BASE	UINT64_C(0x00100000000)
#define PBM_PCI_MEM_SIZE	UINT64_C(0x00100000000)

#define PBM_OBIO_BASE		UINT64_C(0)
#define PBM_OBIO_SIZE		UINT64_C(0x1898)


typedef struct sun4u_fun {
	hw_resource_list_t hw_resources;
	pio_window_t pio_window;
} sun4u_fun_t;

static errno_t sun4u_dev_add(ddf_dev_t *dev);
static void sun4u_init(void);

/** The root device driver's standard operations. */
static driver_ops_t sun4u_ops = {
	.dev_add = &sun4u_dev_add
};

/** The root device driver structure. */
static driver_t sun4u_driver = {
	.name = NAME,
	.driver_ops = &sun4u_ops
};

static hw_resource_t obio_res[] = {
	{
		.type = MEM_RANGE,
		.res.mem_range = {
			.address = PBM_BASE + PBM_OBIO_BASE,
			.size = PBM_OBIO_SIZE,
			.relative = false,
			.endianness = LITTLE_ENDIAN
		}
	}
};

static sun4u_fun_t obio_data = {
	.hw_resources = {
		.count = sizeof(obio_res) / sizeof(obio_res[0]),
		.resources = obio_res
	},
	.pio_window = {
		.mem = {
			.base = PBM_BASE + PBM_OBIO_BASE,
			.size = PBM_OBIO_SIZE
		}
	}
};

static hw_resource_t pci_conf_regs[] = {
	{
		.type = MEM_RANGE,
		.res.mem_range = {
			.address = PBM_BASE + PBM_PCI_CONFIG_BASE,
			.size = PBM_PCI_CONFIG_SIZE,
			.relative = false,
			.endianness = LITTLE_ENDIAN
		}
	}
};

static sun4u_fun_t pci_data = {
	.hw_resources = {
		.count = sizeof(pci_conf_regs) / sizeof(pci_conf_regs[0]),
		.resources = pci_conf_regs
	},
	.pio_window = {
		.mem = {
			.base = PBM_BASE + PBM_PCI_MEM_BASE,
			.size = PBM_PCI_MEM_SIZE
		},
		.io = {
			.base = PBM_BASE + PBM_PCI_IO_BASE,
			.size = PBM_PCI_IO_SIZE
		}
	}
};

/** Obtain function soft-state from DDF function node */
static sun4u_fun_t *sun4u_fun(ddf_fun_t *fnode)
{
	return ddf_fun_data_get(fnode);
}

static hw_resource_list_t *sun4u_get_resources(ddf_fun_t *fnode)
{
	sun4u_fun_t *fun = sun4u_fun(fnode);
	
	assert(fun != NULL);
	return &fun->hw_resources;
}

static errno_t sun4u_enable_interrupt(ddf_fun_t *fun, int irq)
{
	/* TODO */
	
	return false;
}

static pio_window_t *sun4u_get_pio_window(ddf_fun_t *fnode)
{
	sun4u_fun_t *fun = sun4u_fun(fnode);

	assert(fun != NULL);
	return &fun->pio_window;
}

static hw_res_ops_t fun_hw_res_ops = {
	.get_resource_list = &sun4u_get_resources,
	.enable_interrupt = &sun4u_enable_interrupt,
};

static pio_window_ops_t fun_pio_window_ops = {
	.get_pio_window = &sun4u_get_pio_window
};

/* Initialized in sun4u_init() function. */
static ddf_dev_ops_t sun4u_fun_ops;

static bool
sun4u_add_fun(ddf_dev_t *dev, const char *name, const char *str_match_id,
    sun4u_fun_t *fun_proto)
{
	ddf_msg(LVL_DEBUG, "Adding new function '%s'.", name);
	
	ddf_fun_t *fnode = NULL;
	errno_t rc;
	
	/* Create new device. */
	fnode = ddf_fun_create(dev, fun_inner, name);
	if (fnode == NULL)
		goto failure;
	
	sun4u_fun_t *fun = ddf_fun_data_alloc(fnode, sizeof(sun4u_fun_t));
	*fun = *fun_proto;
	
	/* Add match ID */
	rc = ddf_fun_add_match_id(fnode, str_match_id, 100);
	if (rc != EOK)
		goto failure;
	
	/* Set provided operations to the device. */
	ddf_fun_set_ops(fnode, &sun4u_fun_ops);
	
	/* Register function. */
	if (ddf_fun_bind(fnode) != EOK) {
		ddf_msg(LVL_ERROR, "Failed binding function %s.", name);
		goto failure;
	}
	
	return true;
	
failure:
	if (fnode != NULL)
		ddf_fun_destroy(fnode);
	
	ddf_msg(LVL_ERROR, "Failed adding function '%s'.", name);
	
	return false;
}

static bool sun4u_add_functions(ddf_dev_t *dev)
{
	if (!sun4u_add_fun(dev, "obio", "ebus/obio", &obio_data))
		return false;

	return sun4u_add_fun(dev, "pci0", "intel_pci", &pci_data);
}

/** Get the root device.
 *
 * @param dev		The device which is root of the whole device tree (both
 *			of HW and pseudo devices).
 * @return		Zero on success, error number otherwise.
 */
static errno_t sun4u_dev_add(ddf_dev_t *dev)
{
	ddf_msg(LVL_DEBUG, "sun4u_dev_add, device handle = %d",
	    (int)ddf_dev_get_handle(dev));

	/* Register functions. */
	if (!sun4u_add_functions(dev)) {
		ddf_msg(LVL_ERROR, "Failed to add functions for the Malta platform.");
	}
	
	return EOK;
}

static void sun4u_init(void)
{
	ddf_log_init(NAME);
	sun4u_fun_ops.interfaces[HW_RES_DEV_IFACE] = &fun_hw_res_ops;
	sun4u_fun_ops.interfaces[PIO_WINDOW_DEV_IFACE] = &fun_pio_window_ops;
}

int main(int argc, char *argv[])
{
	printf(NAME ": HelenOS sun4u platform driver\n");
	sun4u_init();
	return ddf_driver_main(&sun4u_driver);
}

/**
 * @}
 */
