#ifndef __SD_HANDLER_H
#define __SD_HANDLER_H

#include "lvgl_event.h"

/* Save-text event handler — register with lvgl_evt_register() */
void save_text_handler(const struct lvgl_msg_s *msg);

#endif /* __SD_HANDLER_H */
