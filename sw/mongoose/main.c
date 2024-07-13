// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "mongoose.h"
#include "net.h"

#include "picowota/reboot.h"


#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define TEST_TASK_STACK_SIZE			(( configSTACK_DEPTH_TYPE ) 2048)

static struct mg_mgr mgr;

bool should_do_ota = false;

void main_task(__unused void *params) {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        exit(1);
    } else {
        printf("Connected.\n");
    }

    mg_mgr_init(&mgr);
    web_init(&mgr);

    while(true) {
        mg_mgr_poll(&mgr, 10);
    }

    cyw43_arch_deinit();
}

extern uint16_t framebuffer[];

void panel_task() {
    int i = 0;
    while(true) {
        if(i < 128*128) { 
            framebuffer[i++] = 0xffff;
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

void ota_task() {
    while(true) {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        if(should_do_ota) {
            vTaskDelay(1000/portTICK_PERIOD_MS);
            picowota_reboot(true);
        }
    }
}

void vLaunch( void) {
    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY +1, &task);
    xTaskCreate(panel_task, "PanelTask", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);
    xTaskCreate(ota_task, "OtaTask", 256, NULL, TEST_TASK_PRIORITY, &task);
    vTaskStartScheduler();
}
