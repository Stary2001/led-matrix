// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "mongoose.h"
#include "net.h"
#include "panel.h"

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


#include "../font8x8_basic.h"
panel_message current_message = {.type = MESSAGE_NONE, .text_message={}};

void clear_framebuffer() {
    memset(framebuffer, 0, WIDTH * HEIGHT * 2);
}

void draw_char(char chr, size_t xx, size_t yy) {
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            if(font8x8_basic[chr][y] & (1<<x)) {
                framebuffer[(yy + y) * WIDTH + xx + x] = 0xffff;
            } else {
                framebuffer[(yy + y) * WIDTH + xx + x] = 0;
            }
        }
    }
}

void panel_task() {
    int i = 0;
    while(true) {
        if(current_message.type != MESSAGE_NONE) {
            switch(current_message.type) {
                case MESSAGE_TEXT:
                {
                    panel_text_message *text_message = &current_message.text_message;
                    if(text_message->clear) {
                        clear_framebuffer();
                    }

                    size_t len = strlen(text_message->message);
                    size_t xx = 0;
                    size_t yy = 0;

                    for(size_t i = 0; i < len; i++) {
                        draw_char(text_message->message[i], xx, yy);
                        xx += 8;
                        if(xx == 128) {
                            xx = 0;
                            yy += 8;
                        }
                        vTaskDelay(100/portTICK_PERIOD_MS);
                    }
                    vTaskDelay(1000/portTICK_PERIOD_MS);

                    current_message.type = MESSAGE_NONE;
                }
                break;

                default:
                // idk
                break;
            }
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
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
