#ifndef __LED_HANDLER_H
#define __LED_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <nuttx/myled/myled.h>
#include "lvgl_event.h"

#define LED_COUNT  3

void led_state_init(void);
int lvgl_event_send_set_led(uint8_t led_num, bool on);
void set_led_handler(const struct lvgl_msg_s *msg);
void save_led_status_handler(const struct lvgl_msg_s *msg);

#endif /* __LED_HANDLER_H */
