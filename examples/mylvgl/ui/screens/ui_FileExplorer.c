// File Explorer screen for browsing SD card files and directories

#include "../ui.h"
#include "nsh_terminal.h"
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arch/board/board_paths.h>

#define MAX_ENTRIES    128
#define ENTRY_NAME_MAX 64
#define CONTENT_MAX    1024

lv_obj_t *ui_FileExplorer = NULL;

static lv_obj_t *ui_btnHome = NULL;
static lv_obj_t *ui_btnBack = NULL;
static lv_obj_t *ui_labelPath = NULL;
static lv_obj_t *ui_panelFiles = NULL;
static lv_obj_t *ui_labelContent = NULL;
static lv_obj_t *ui_labelNoSd = NULL;

static char g_current_path[256];
static char g_entry_names[MAX_ENTRIES][ENTRY_NAME_MAX];
static int  g_entry_count;

/****************************************************************************
 * Forward declarations
 ****************************************************************************/

static void scan_directory(void);

/****************************************************************************
 * Callbacks
 ****************************************************************************/

static void home_click_cb(lv_event_t *e)
{
    _ui_screen_change(&ui_HomeScreen, LV_SCR_LOAD_ANIM_NONE,
                      0, 0, &ui_HomeScreen_screen_init);
}

static void back_click_cb(lv_event_t *e)
{
    char *slash = strrchr(g_current_path, '/');
    if (slash != NULL && slash != g_current_path)
    {
        *slash = '\0';
    }
    else if (slash == g_current_path)
    {
        g_current_path[1] = '\0';
    }

    if (strcmp(g_current_path, SD_BASE_DIR) == 0)
    {
        lv_obj_add_flag(ui_btnBack, LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text(ui_labelPath, g_current_path);
    lv_textarea_set_text(ui_labelContent, "");
    scan_directory();
}

static void dir_click_cb(lv_event_t *e)
{
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);

    size_t len = strlen(g_current_path);
    snprintf(g_current_path + len, sizeof(g_current_path) - len,
             "/%s", g_entry_names[idx]);

    lv_label_set_text(ui_labelPath, g_current_path);
    lv_obj_clear_flag(ui_btnBack, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(ui_labelContent, "");
    scan_directory();
}

static void file_click_cb(lv_event_t *e)
{
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s",
             g_current_path, g_entry_names[idx]);

    FILE *fp = fopen(fullpath, "r");
    if (!fp)
    {
        lv_textarea_set_text(ui_labelContent, "Failed to open file.");
        return;
    }

    char content[CONTENT_MAX + 1];
    size_t nread = fread(content, 1, CONTENT_MAX, fp);
    fclose(fp);
    content[nread] = '\0';
    lv_textarea_set_text(ui_labelContent, content);
}

/****************************************************************************
 * Directory scanner
 ****************************************************************************/

static void scan_directory(void)
{
    lv_obj_clean(ui_panelFiles);

    DIR *dir = opendir(g_current_path);
    if (!dir)
    {
        return;
    }

    struct stat statbuf;
    char fullpath[PATH_MAX];
    struct dirent *entry;

    g_entry_count = 0;

    /* First pass: directories */
    while ((entry = readdir(dir)) != NULL && g_entry_count < MAX_ENTRIES)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s",
                 g_current_path, entry->d_name);

        if (stat(fullpath, &statbuf) < 0)
            continue;

        if (!S_ISDIR(statbuf.st_mode))
            continue;

        strncpy(g_entry_names[g_entry_count], entry->d_name,
                ENTRY_NAME_MAX - 1);
        g_entry_names[g_entry_count][ENTRY_NAME_MAX - 1] = '\0';

        char label_text[ENTRY_NAME_MAX + 4];
        snprintf(label_text, sizeof(label_text), "  %s/", entry->d_name);

        lv_obj_t *btn = lv_button_create(ui_panelFiles);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3AACB7),
                                  LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 4, LV_PART_MAIN);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, label_text);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, dir_click_cb, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)g_entry_count);

        g_entry_count++;
    }

    rewinddir(dir);

    /* Second pass: files */
    while ((entry = readdir(dir)) != NULL && g_entry_count < MAX_ENTRIES)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s",
                 g_current_path, entry->d_name);

        if (stat(fullpath, &statbuf) < 0)
            continue;

        if (!S_ISREG(statbuf.st_mode))
            continue;

        strncpy(g_entry_names[g_entry_count], entry->d_name,
                ENTRY_NAME_MAX - 1);
        g_entry_names[g_entry_count][ENTRY_NAME_MAX - 1] = '\0';

        lv_obj_t *btn = lv_button_create(ui_panelFiles);
        lv_obj_set_width(btn, LV_PCT(100));
        lv_obj_set_height(btn, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x8E8585),
                                  LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 4, LV_PART_MAIN);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, entry->d_name);
        lv_obj_center(label);

        lv_obj_add_event_cb(btn, file_click_cb, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)g_entry_count);

        g_entry_count++;
    }

    closedir(dir);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void ui_FileExplorer_screen_init(void)
{
    ui_FileExplorer = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_FileExplorer, LV_OBJ_FLAG_SCROLLABLE);

    /* Home button */
    ui_btnHome = lv_button_create(ui_FileExplorer);
    lv_obj_set_size(ui_btnHome, 80, 36);
    lv_obj_align(ui_btnHome, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_style_bg_color(ui_btnHome, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_set_style_radius(ui_btnHome, 6, LV_PART_MAIN);
    lv_obj_t *home_lbl = lv_label_create(ui_btnHome);
    lv_label_set_text(home_lbl, "⌂ Home");
    lv_obj_center(home_lbl);
    lv_obj_add_event_cb(ui_btnHome, home_click_cb, LV_EVENT_CLICKED, NULL);

    /* Path label */
    ui_labelPath = lv_label_create(ui_FileExplorer);
    lv_label_set_text(ui_labelPath, SD_BASE_DIR);
    lv_obj_align(ui_labelPath, LV_ALIGN_TOP_LEFT, 96, 14);

    /* Back button (hidden at root) */
    ui_btnBack = lv_button_create(ui_FileExplorer);
    lv_obj_set_size(ui_btnBack, 44, 28);
    lv_obj_align(ui_btnBack, LV_ALIGN_TOP_RIGHT, -8, 10);
    lv_obj_set_style_bg_color(ui_btnBack, lv_color_hex(0xE05050),
                              LV_PART_MAIN);
    lv_obj_set_style_radius(ui_btnBack, 6, LV_PART_MAIN);
    lv_obj_add_flag(ui_btnBack, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *back_lbl = lv_label_create(ui_btnBack);
    lv_label_set_text(back_lbl, "<--");
    lv_obj_center(back_lbl);
    lv_obj_add_event_cb(ui_btnBack, back_click_cb, LV_EVENT_CLICKED, NULL);

    /* Scrollable file list panel */
    ui_panelFiles = lv_obj_create(ui_FileExplorer);
    lv_obj_set_width(ui_panelFiles, LV_PCT(100));
    lv_obj_set_height(ui_panelFiles, 192);
    lv_obj_align(ui_panelFiles, LV_ALIGN_TOP_LEFT, 0, 48);
    lv_obj_set_flex_flow(ui_panelFiles, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ui_panelFiles, 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_panelFiles, 4, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(ui_panelFiles, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(ui_panelFiles, 0, LV_PART_MAIN);

    /* File content textarea (bottom half of screen) */
    ui_labelContent = lv_textarea_create(ui_FileExplorer);
    lv_obj_set_width(ui_labelContent, LV_PCT(100));
    lv_obj_set_height(ui_labelContent, 240);
    lv_obj_align(ui_labelContent, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_textarea_set_text(ui_labelContent, "");
    lv_obj_remove_flag(ui_labelContent, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_bg_color(ui_labelContent, lv_color_hex(0xF0F0F0),
                              LV_PART_MAIN);

    /* No SD Card message */
    ui_labelNoSd = lv_label_create(ui_FileExplorer);
    lv_label_set_text(ui_labelNoSd, "No SD Card");
    lv_obj_center(ui_labelNoSd);
    lv_obj_set_style_text_font(ui_labelNoSd, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_labelNoSd, lv_color_hex(0xE05050),
                                LV_PART_MAIN);
    lv_obj_add_flag(ui_labelNoSd, LV_OBJ_FLAG_HIDDEN);

    /* Initial scan */
    struct stat statbuf;
    if (stat(SD_BASE_DIR, &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
    {
        strncpy(g_current_path, SD_BASE_DIR, sizeof(g_current_path) - 1);
        g_current_path[sizeof(g_current_path) - 1] = '\0';
        lv_label_set_text(ui_labelPath, g_current_path);
        lv_obj_add_flag(ui_labelNoSd, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_panelFiles, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_labelContent, LV_OBJ_FLAG_HIDDEN);
        scan_directory();
    }
    else
    {
        strncpy(g_current_path, SD_BASE_DIR, sizeof(g_current_path) - 1);
        g_current_path[sizeof(g_current_path) - 1] = '\0';
        lv_label_set_text(ui_labelPath, g_current_path);
        lv_obj_clear_flag(ui_labelNoSd, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_panelFiles, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_labelContent, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_btnBack, LV_OBJ_FLAG_HIDDEN);
    }

    nsh_terminal_toggle_btn_create(ui_FileExplorer);
}

void ui_FileExplorer_screen_destroy(void)
{
    if (ui_FileExplorer)
        lv_obj_del(ui_FileExplorer);

    ui_FileExplorer = NULL;
    ui_btnHome = NULL;
    ui_btnBack = NULL;
    ui_labelPath = NULL;
    ui_panelFiles = NULL;
    ui_labelContent = NULL;
    ui_labelNoSd = NULL;
}
