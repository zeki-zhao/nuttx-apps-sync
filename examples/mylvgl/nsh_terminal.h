#ifndef __NSH_TERMINAL_H
#define __NSH_TERMINAL_H

#include <lvgl/lvgl.h>

#define TERMINAL_MAX_BYTES  2048

/* Terminal UI helpers (defined in ui/screens/ui_Terminal.c) */
void term_label_append(lv_obj_t *label, const char *txt);
void term_scroll_bottom(lv_obj_t *obj);
void term_backspace(lv_obj_t *label);
void trim_textarea(lv_obj_t *ta);
void term_output_click_cb(lv_event_t *e);

int nsh_terminal_init(void);
int nsh_terminal_open(void);
int nsh_terminal_close(void);
void nsh_terminal_toggle_btn_create(lv_obj_t *parent);

#endif /* __NSH_TERMINAL_H */
