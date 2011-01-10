/*
 * Copyright (c) 2010 Lenka Trochtova
 * Copyright (c) 2010 Vojtech Horky
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
 * @defgroup root Root device driver.
 * @brief HelenOS root device driver.
 * @{
 */

/** @file
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <bool.h>
#include <fibril_synch.h>
#include <stdlib.h>
#include <str.h>
#include <ctype.h>
#include <macros.h>
#include <inttypes.h>
#include <sysinfo.h>

#include <driver.h>
#include <devman.h>
#include <ipc/devman.h>

#define NAME "root"

#define PLATFORM_DEVICE_NAME "hw"
#define PLATFORM_DEVICE_MATCH_ID_FMT "platform/%s"
#define PLATFORM_DEVICE_MATCH_SCORE 100

#define VIRTUAL_DEVICE_NAME "virt"
#define VIRTUAL_DEVICE_MATCH_ID "rootvirt"
#define VIRTUAL_DEVICE_MATCH_SCORE 100

static int root_add_device(device_t *dev);

/** The root device driver's standard operations. */
static driver_ops_t root_ops = {
	.add_device = &root_add_device
};

/** The root device driver structure. */
static driver_t root_driver = {
	.name = NAME,
	.driver_ops = &root_ops
};

/** Create the device which represents the root of virtual device tree.
 *
 * @param parent Parent of the newly created device.
 * @return Error code.
 */
static int add_virtual_root_child(device_t *parent)
{
	printf(NAME ": adding new child for virtual devices.\n");
	printf(NAME ":   device node is `%s' (%d %s)\n", VIRTUAL_DEVICE_NAME,
	    VIRTUAL_DEVICE_MATCH_SCORE, VIRTUAL_DEVICE_MATCH_ID);

	int res = child_device_register_wrapper(parent, VIRTUAL_DEVICE_NAME,
	    VIRTUAL_DEVICE_MATCH_ID, VIRTUAL_DEVICE_MATCH_SCORE);

	return res;
}

/** Create the device which represents the root of HW device tree.
 *
 * @param parent	Parent of the newly created device.
 * @return 0 on success, negative error number otherwise.
 */
static int add_platform_child(device_t *parent)
{
	char *match_id;
	char *platform;
	size_t platform_size;
	int res;

	/* Get platform name from sysinfo. */

	platform = sysinfo_get_data("platform", &platform_size);
	if (platform == NULL) {
		printf(NAME ": Failed to obtain platform name.\n");
		return ENOENT;
	}

	/* Null-terminate string. */
	platform = realloc(platform, platform_size + 1);
	if (platform == NULL) {
		printf(NAME ": Memory allocation failed.\n");
		return ENOMEM;
	}

	platform[platform_size] = '\0';

	/* Construct match ID. */

	if (asprintf(&match_id, PLATFORM_DEVICE_MATCH_ID_FMT, platform) == -1) {
		printf(NAME ": Memory allocation failed.\n");
		return ENOMEM;
	}

	/* Add child. */

	printf(NAME ": adding new child for platform device.\n");
	printf(NAME ":   device node is `%s' (%d %s)\n", PLATFORM_DEVICE_NAME,
	    PLATFORM_DEVICE_MATCH_SCORE, match_id);

	res = child_device_register_wrapper(parent, PLATFORM_DEVICE_NAME,
	    match_id, PLATFORM_DEVICE_MATCH_SCORE);

	return res;
}

/** Get the root device.
 *
 * @param dev		The device which is root of the whole device tree (both
 *			of HW and pseudo devices).
 */
static int root_add_device(device_t *dev)
{
	printf(NAME ": root_add_device, device handle=%" PRIun "\n",
	    dev->handle);
	
	/*
	 * Register virtual devices root.
	 * We ignore error occurrence because virtual devices shall not be
	 * vital for the system.
	 */
	add_virtual_root_child(dev);

	/* Register root device's children. */
	int res = add_platform_child(dev);
	if (EOK != res)
		printf(NAME ": failed to add child device for platform.\n");
	
	return res;
}

int main(int argc, char *argv[])
{
	printf(NAME ": HelenOS root device driver\n");
	return driver_main(&root_driver);
}

/**
 * @}
 */
