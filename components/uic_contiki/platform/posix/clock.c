/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: clock.c,v 1.3 2009/09/09 21:11:24 adamdunkels Exp $
 */

/**
 * \file
 *         Clock implementation for Unix.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */


#ifdef __linux__
#define _POSIX_C_SOURCE 199309L
#include "sys/clock.h"
#include <time.h>

//Fix some bad lib c
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 4
#endif
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW,&ts);

  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW,&ts);

  return ts.tv_sec;
}
#else
#include "sys/clock.h"
#include <sys/time.h>

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec;

}

#endif
/*---------------------------------------------------------------------------*/

int clock_lte(clock_time_t t1, clock_time_t t2) {
  clock_time_t diff = t1 - t2;
  if (diff == 0) {
    return 1;
  }
  /* If t1 < t2 return true, but if it has crossed the half-space,
   * return t2 < t1.  */
  clock_time_t inverse_diff = t2 - t1;
  return (inverse_diff < diff);
}

void
clock_delay(unsigned int d)
{
  /* Does not do anything. */
}
