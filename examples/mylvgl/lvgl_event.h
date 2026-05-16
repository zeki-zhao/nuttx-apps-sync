#ifndef __LVGL_EVENT_H
#define __LVGL_EVENT_H

#include <stdbool.h>
#include <stdint.h>
#include <mqueue.h>
#include <pthread.h>

#define LVGL_EVENT_MQ_NAME  "/lvgl_evt"

#define LVGL_EVT_MAX_HANDLERS  8

#define LVGL_MSG_SET_LED     1
#define LVGL_MSG_SAVE_LED_STATUS   3
#define LVGL_MSG_SAVE_TEXT   2


struct lvgl_msg_s
{
    long type;
    union
    {
        struct {
            uint8_t led_num;
            bool led_on;
        };
        char *text;
    };
};

typedef void (*lvgl_evt_handler_t)(const struct lvgl_msg_s *msg);

mqd_t lvgl_event_init(void);
void lvgl_event_fini(void);
int lvgl_evt_register(long type, lvgl_evt_handler_t handler);
void lvgl_evt_unregister(long type);
int lvgl_event_send_set_led(uint8_t led_num, bool on);
int lvgl_event_send_text(const char *text);
mqd_t lvgl_event_get_mqd(const char *name);
pthread_addr_t LvglEventProcess(pthread_addr_t arg);

#endif /* __LVGL_EVENT_H */
