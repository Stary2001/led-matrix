/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
extern void do_hub75();
extern void vLaunch();

int main() {
    stdio_init_all();
    set_sys_clock_khz(250000, false);
    multicore_launch_core1(do_hub75);
 
    vLaunch();

    while(true) {
         sleep_ms(1000);
    }
}