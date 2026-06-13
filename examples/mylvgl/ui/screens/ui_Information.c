// Information screen: version info and firmware update
#include "../ui.h"
#include "nsh_terminal.h"
#include "lvgl_event.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

extern volatile bool g_upgrade_busy;

#define UPDATE_DIR "/mnt/sd/firmware/update"

lv_obj_t *ui_Information = NULL;

static lv_obj_t *ui_btnHome = NULL;
lv_obj_t *ui_labelVersion = NULL;
lv_obj_t *ui_labelUptime = NULL;
lv_obj_t *ui_labelUptimePrefix = NULL;
static lv_obj_t *ui_btnUpdate = NULL;
static lv_timer_t *ui_update_timer = NULL;

/****************************************************************************
 * Forward declarations
 ****************************************************************************/

static void check_update_dir(lv_timer_t *timer);
static void refresh_uptime(lv_timer_t *timer);

/****************************************************************************
 * Callbacks
 ****************************************************************************/

static void home_click_cb(lv_event_t *e)
{
    _ui_screen_change(&ui_HomeScreen, LV_SCR_LOAD_ANIM_NONE,
                      0, 0, &ui_HomeScreen_screen_init);
}

static void update_click_cb(lv_event_t *e)
{
    g_upgrade_busy = true;
    lvgl_event_send_upgrade(2);
}

/****************************************************************************
 * Read /proc/version into a buffer
 ****************************************************************************/

static void load_version_info(void)
{
    FILE *fp = fopen("/proc/version", "r");
    if (!fp)
    {
        lv_label_set_text(ui_labelVersion, "Cannot read /proc/version");
        return;
    }

    char buf[200];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = '\0';

    /* Remove trailing newline */
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';

    lv_label_set_text(ui_labelVersion, buf);
}

/****************************************************************************
 * Timer: check for .bin files every 1s
 ****************************************************************************/

static void check_update_dir(lv_timer_t *timer)
{
    if (lv_scr_act() != ui_Information)
        return;

    bool have_nuttx   = false;
    DIR *dir = opendir(UPDATE_DIR);
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, "nuttx.img") == 0)
                have_nuttx = true;
        }
        closedir(dir);
    }

    bool enable = have_nuttx && !g_upgrade_busy;

    /* Update button */
    lv_obj_set_style_bg_color(ui_btnUpdate,
        enable ? lv_color_hex(0xFF8800) : lv_color_hex(0x999999),
        LV_PART_MAIN);
    if (enable)
        lv_obj_add_flag(ui_btnUpdate, LV_OBJ_FLAG_CLICKABLE);
    else
        lv_obj_remove_flag(ui_btnUpdate, LV_OBJ_FLAG_CLICKABLE);
}

static void refresh_uptime(lv_timer_t *timer)
{
    if (lv_scr_act() != ui_Information)
        return;

    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp)
        return;

    char buf[40];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = '\0';

    /* Remove trailing newline */
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';

    lv_label_set_text(ui_labelUptime, buf);
}

/****************************************************************************
 * Screen init
 ****************************************************************************/

void ui_Information_screen_init(void)
{
    ui_Information = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_Information, LV_OBJ_FLAG_SCROLLABLE);

    /* Home button */
    ui_btnHome = lv_button_create(ui_Information);
    lv_obj_set_size(ui_btnHome, 90, 40);
    lv_obj_align(ui_btnHome, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(ui_btnHome, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_btnHome, 255, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_btnHome, 8, LV_PART_MAIN);
    lv_obj_t *home_lbl = lv_label_create(ui_btnHome);
    lv_label_set_text(home_lbl, "Home");
    lv_obj_center(home_lbl);
    lv_obj_add_event_cb(ui_btnHome, home_click_cb, LV_EVENT_CLICKED, NULL);

    /* Uptime: static "Running time: " + dynamic value */
    ui_labelUptimePrefix = lv_label_create(ui_Information);
    lv_label_set_text(ui_labelUptimePrefix, "Running time: ");
    lv_obj_set_style_text_color(ui_labelUptimePrefix, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_labelUptimePrefix, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(ui_labelUptimePrefix, LV_ALIGN_TOP_MID, -60, 70);

    ui_labelUptime = lv_label_create(ui_Information);
    lv_label_set_text(ui_labelUptime, "");
    lv_obj_set_style_text_color(ui_labelUptime, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_labelUptime, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align_to(ui_labelUptime, ui_labelUptimePrefix, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    /* Version info label (half width, below uptime) */
    ui_labelVersion = lv_label_create(ui_Information);
    lv_label_set_text(ui_labelVersion, "Loading...");
    lv_obj_set_width(ui_labelVersion, 380);
    lv_obj_align(ui_labelVersion, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_style_text_color(ui_labelVersion, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_align(ui_labelVersion, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(ui_labelVersion, &lv_font_montserrat_14,
                               LV_PART_MAIN);

    /* Update button */
    ui_btnUpdate = lv_button_create(ui_Information);
    lv_obj_set_size(ui_btnUpdate, 200, 50);
    lv_obj_align(ui_btnUpdate, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_bg_color(ui_btnUpdate, lv_color_hex(0x999999),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_btnUpdate, 255, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_btnUpdate, 10, LV_PART_MAIN);
    lv_obj_t *update_lbl = lv_label_create(ui_btnUpdate);
    lv_label_set_text(update_lbl, "Update");
    lv_obj_center(update_lbl);
    lv_obj_add_event_cb(ui_btnUpdate, update_click_cb, LV_EVENT_CLICKED, NULL);

    /* Load version info */
    load_version_info();

    /* Timer: check for update .bin files every 1s */
    ui_update_timer = lv_timer_create(check_update_dir, 1000, NULL);
    lv_timer_create(refresh_uptime, 10, NULL);
}

/****************************************************************************
 * Screen destroy
 ****************************************************************************/

void ui_Information_screen_destroy(void)
{
    if (ui_update_timer)
        lv_timer_del(ui_update_timer);
    ui_update_timer = NULL;

    if (ui_Information)
        lv_obj_del(ui_Information);

    ui_Information = NULL;
    ui_btnHome = NULL;
    ui_labelVersion = NULL;
    ui_labelUptime = NULL;
    ui_labelUptimePrefix = NULL;
    ui_btnUpdate = NULL;
}
