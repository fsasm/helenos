/*
 * Copyright (c) 2012 Maurizio Lombardi
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
 * @defgroup CMOS RTC driver.
 * @brief HelenOS RTC driver.
 * @{
 */

/** @file
 */

#include <errno.h>
#include <ddi.h>
#include <stdio.h>
#include <ddf/driver.h>
#include <ddf/log.h>
#include <ops/clock.h>

#define NAME "RTC"

static int
rtc_time_get(ddf_fun_t *fun, time_t *t);

static int
rtc_time_set(ddf_fun_t *fun, time_t t);


static ddf_dev_ops_t rtc_dev_ops;

static driver_t rtc_driver = {
	.name = NAME,
	.driver_ops = NULL,
};

static clock_dev_ops_t rtc_clock_dev_ops = {
	.time_get = rtc_time_get,
	.time_set = rtc_time_set,
};

typedef struct rtc {
	/** DDF device node */
	ddf_dev_t *dev;
	/** DDF function node */
	ddf_fun_t *fun;
} rtc_t;


/** Initialize the RTC driver */
static void
rtc_init(void)
{
	ddf_log_init(NAME, LVL_ERROR);

	rtc_dev_ops.open = NULL;
	rtc_dev_ops.close = NULL;

	rtc_dev_ops.interfaces[CLOCK_DEV_IFACE] = &rtc_clock_dev_ops;
	rtc_dev_ops.default_handler = NULL;
}

static int
rtc_time_get(ddf_fun_t *fun, time_t *t)
{
	return EOK;
}

static int
rtc_time_set(ddf_fun_t *fun, time_t t)
{
	return EOK;
}

int
main(int argc, char **argv)
{
	printf(NAME ": HelenOS RTC driver\n");
	rtc_init();
	return ddf_driver_main(&rtc_driver);
}

/**
 * @}
 */
