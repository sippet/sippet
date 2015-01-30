/* $Id: os_timestamp_posix.c 3553 2011-05-05 06:14:19Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/os.h>
#include <pj/errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(PJ_HAS_UNISTD_H) && PJ_HAS_UNISTD_H != 0
#   include <unistd.h>

#   if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 && \
       defined(_POSIX_MONOTONIC_CLOCK)
#       define USE_POSIX_TIMERS 1
#   endif

#endif

#include <mach/mach.h>
#include <mach/clock.h>
#include <errno.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC	1000000000
#endif

PJ_DEF(pj_status_t) pj_get_timestamp(pj_timestamp *ts)
{
    mach_timespec_t tp;
    int ret;
    clock_serv_t serv;

    ret = host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &serv);
    if (ret != KERN_SUCCESS) {
	return PJ_RETURN_OS_ERROR(EINVAL);
    }

    ret = clock_get_time(serv, &tp);
    if (ret != KERN_SUCCESS) {
	return PJ_RETURN_OS_ERROR(EINVAL);
    }

    ts->u64 = tp.tv_sec;
    ts->u64 *= NSEC_PER_SEC;
    ts->u64 += tp.tv_nsec;

    return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pj_get_timestamp_freq(pj_timestamp *freq)
{
    freq->u32.hi = 0;
    freq->u32.lo = NSEC_PER_SEC;

    return PJ_SUCCESS;
}

