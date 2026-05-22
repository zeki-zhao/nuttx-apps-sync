#include "../ui.h"
#include "nsh_terminal.h"

lv_obj_t *ui_Terminal = NULL;
lv_obj_t *ui_TerminalOutput = NULL;
lv_obj_t *ui_TerminalInput = NULL;
lv_obj_t *ui_TerminalKeyboard = NULL;

static void close_btn_callback(lv_event_t *e)
{
    nsh_terminal_close();
}

static void open_btn_callback(lv_event_t *e)
{
    nsh_terminal_open();
}

static void add_label_to_btn(lv_obj_t *btn, const char *text)
{
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}

void ui_Terminal_screen_init(void)
{
    ui_Terminal = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_Terminal, lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN);

    /* Monospaced font style in black */
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_unscii_8);
    lv_style_set_text_color(&style, lv_color_hex(0x000000));

    /* Output textarea (read-only, scrollable) */
    ui_TerminalOutput = lv_textarea_create(ui_Terminal);
    lv_obj_add_style(ui_TerminalOutput, &style, 0);
    lv_obj_set_width(ui_TerminalOutput, LV_PCT(100));
    lv_obj_set_height(ui_TerminalOutput, LV_PCT(65));
    lv_obj_align(ui_TerminalOutput, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_remove_flag(ui_TerminalOutput,
                       LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);

    /* Input textarea (bottom area, above keyboard) */
    ui_TerminalInput = lv_textarea_create(ui_Terminal);
    lv_obj_add_style(ui_TerminalInput, &style, 0);
    lv_obj_set_width(ui_TerminalInput, LV_PCT(100));
    lv_obj_set_height(ui_TerminalInput, LV_SIZE_CONTENT);
    lv_obj_align(ui_TerminalInput, LV_ALIGN_BOTTOM_MID, 0, -180);
    lv_obj_set_style_bg_color(ui_TerminalInput, lv_color_hex(0xEEEEEE),
                              LV_PART_MAIN);

    /* Keyboard */
    ui_TerminalKeyboard = lv_keyboard_create(ui_Terminal);
    lv_obj_add_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(ui_TerminalKeyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(ui_TerminalKeyboard, ui_TerminalInput);

    /* Close button (circular, top-right) */
    {
        lv_obj_t *btn = lv_button_create(ui_Terminal);
        lv_obj_set_size(btn, 44, 44);
        lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -10, 10);
        lv_obj_set_style_radius(btn, 22, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE05050),
                                  LV_PART_MAIN);
        lv_obj_add_event_cb(btn, close_btn_callback,
                            LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, "X");
        lv_obj_center(label);
    }
}

void ui_Terminal_homebutton_create(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_width(btn, 160);
    lv_obj_set_height(btn, 60);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 160);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3AACB7), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, open_btn_callback, LV_EVENT_CLICKED, NULL);
    add_label_to_btn(btn, "Terminal");
}
