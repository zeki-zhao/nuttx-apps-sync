#ifndef __LED_HANDLER_H
#define __LED_HANDLER_H

#include "lvgl_event.h"

/* LED toggle event handler — register with lvgl_evt_register() */
void toggle_led_handler(const struct lvgl_msg_s *msg);

#endif /* __LED_HANDLER_H */
