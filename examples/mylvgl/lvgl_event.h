#ifndef __LVGL_EVENT_H
#define __LVGL_EVENT_H

#include <stdint.h>
#include <mqueue.h>
#include <pthread.h>

#define LVGL_EVT_MAX_HANDLERS  8

#define LVGL_MSG_TOGGLE_LED  1
#define LVGL_MSG_SAVE_TEXT   2

struct lvgl_msg_s
{
    long type;
    union
    {
        uint8_t led_num;   /* for TOGGLE_LED */
        char *text;        /* for SAVE_TEXT — strdup'd, freed by handler */
    };
};

typedef void (*lvgl_evt_handler_t)(const struct lvgl_msg_s *msg);

mqd_t lvgl_event_init(void);
void lvgl_event_fini(void);
int lvgl_evt_register(long type, lvgl_evt_handler_t handler);
void lvgl_evt_unregister(long type);
int lvgl_event_send_toggle_led(uint8_t led_num);
int lvgl_event_send_text(const char *text);
pthread_addr_t LvglEventProcess(pthread_addr_t arg);

#endif /* __LVGL_EVENT_H */
