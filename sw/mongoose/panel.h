#pragma once
#include <stdint.h>

typedef enum {
    MESSAGE_NONE,
    MESSAGE_TEXT
} message_type;

typedef struct { 
    const char *message;
    bool clear;
} panel_text_message;

typedef struct {
    message_type type;
    union {
        panel_text_message text_message;
    };
} panel_message;

#define WIDTH 128
#define PANEL_HEIGHT 64
#define HEIGHT 128

extern uint16_t framebuffer[];
extern panel_message current_message;