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
    //sleep_ms(10000);
    for(int i = 0; i < 5; i++) {
         printf("eepy %i\n", i);
         sleep_ms(1000);
        // spin
    }
    vLaunch();

    int count = 0;
    while(true) {
         printf("eepy %i\n", count++);
         sleep_ms(1000);
    }
}