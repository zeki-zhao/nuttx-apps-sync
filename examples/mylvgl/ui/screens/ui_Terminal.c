#include "../ui.h"
#include "nsh_terminal.h"

#include <string.h>

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

    /* Output label (read-only, scrollable) */
    ui_TerminalOutput = lv_label_create(ui_Terminal);
    lv_obj_add_style(ui_TerminalOutput, &style, 0);
    lv_obj_set_width(ui_TerminalOutput, LV_PCT(100));
    lv_label_set_long_mode(ui_TerminalOutput, LV_LABEL_LONG_WRAP);
    lv_obj_set_height(ui_TerminalOutput, LV_PCT(65));
    lv_obj_align(ui_TerminalOutput, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(ui_TerminalOutput, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(ui_TerminalOutput, LV_OBJ_FLAG_CLICKABLE);

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

/****************************************************************************
 * Screen tracking for terminal toggle
 ****************************************************************************/

static lv_obj_t *g_prev_screen;

/****************************************************************************
 * Name: trim_textarea
 *
 * Description:
 *   Trim the textarea text to keep only the last TERMINAL_MAX_BYTES,
 *   preventing unbounded growth that causes redraw slowdowns.
 *
 ****************************************************************************/

void trim_textarea(lv_obj_t *ta)
{
    const char *text = lv_label_get_text(ta);
    size_t len = strlen(text);
    if (len > TERMINAL_MAX_BYTES)
    {
        const char *keep = text + (len - TERMINAL_MAX_BYTES);
        char *tmp = lv_malloc(TERMINAL_MAX_BYTES + 1);
        if (tmp)
        {
            memcpy(tmp, keep, TERMINAL_MAX_BYTES);
            tmp[TERMINAL_MAX_BYTES] = '\0';
            lv_label_set_text(ta, tmp);
            lv_free(tmp);
        }
    }
}

/****************************************************************************
 * Name: term_label_append
 *
 * Description:
 *   Append text to the terminal label (wrapper around lv_label_ins_text).
 *
 ****************************************************************************/

void term_label_append(lv_obj_t *label, const char *txt)
{
    lv_label_ins_text(label, strlen(lv_label_get_text(label)), txt);
}

/****************************************************************************
 * Name: term_scroll_bottom
 ****************************************************************************/

void term_scroll_bottom(lv_obj_t *obj)
{
    lv_obj_scroll_to_y(obj, LV_COORD_MAX, LV_ANIM_OFF);
}

/****************************************************************************
 * Name: term_backspace
 *
 * Description:
 *   Delete the last character from the terminal output label and update
 *   the line buffer.  Used by both physical and virtual keyboard paths.
 *
 ****************************************************************************/

void term_backspace(lv_obj_t *label)
{
    const char *txt = lv_label_get_text(label);
    size_t tlen = strlen(txt);
    if (tlen > 0)
    {
        char *tmp = lv_malloc(tlen);
        if (tmp)
        {
            memcpy(tmp, txt, tlen - 1);
            tmp[tlen - 1] = '\0';
            lv_label_set_text(label, tmp);
            lv_free(tmp);
        }
    }
}

/****************************************************************************
 * Name: term_output_click_cb
 *
 * Description:
 *   Toggle virtual keyboard visibility on output area click.
 *
 ****************************************************************************/

void term_output_click_cb(lv_event_t *e)
{
    if (lv_obj_has_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_clear_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_update_layout(ui_TerminalKeyboard);
        lv_obj_set_height(ui_TerminalOutput,
                          lv_obj_get_y(ui_TerminalKeyboard));
        term_scroll_bottom(ui_TerminalOutput);
    }
    else
    {
        lv_obj_add_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_height(ui_TerminalOutput, LV_PCT(100));
    }
}

/****************************************************************************
 * Name: toggle_btn_cb
 *
 * Description:
 *   Callback for the terminal toggle button.
 *
 ****************************************************************************/

static void toggle_btn_cb(lv_event_t *e)
{
    nsh_terminal_open();
}

/****************************************************************************
 * Name: nsh_terminal_toggle_btn_create
 *
 * Description:
 *   Create a terminal toggle button at the bottom-right corner of the given
 *   parent screen.
 *
 ****************************************************************************/

void nsh_terminal_toggle_btn_create(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 44, 44);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_radius(btn, 22, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3AACB7), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, toggle_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, ">_");
    lv_obj_center(label);
}

/****************************************************************************
 * Name: nsh_terminal_open
 *
 * Description:
 *   Switch to the terminal screen.
 *
 ****************************************************************************/

int nsh_terminal_open(void)
{
    if (ui_Terminal == NULL)
    {
        return -1;
    }

    g_prev_screen = lv_scr_act();
    lv_scr_load(ui_Terminal);
    return 0;
}

/****************************************************************************
 * Name: nsh_terminal_close
 *
 * Description:
 *   Return to the screen that was active before opening the terminal.
 *
 ****************************************************************************/

int nsh_terminal_close(void)
{
    lv_scr_load(g_prev_screen != NULL ? g_prev_screen : ui_HomeScreen);
    return 0;
}
