// Music Player screen — play .txt melody files from SD card
// with sequential, repeat-one, and shuffle modes.

#include "../ui.h"
#include "nsh_terminal.h"
#include "buzzer.h"

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arch/board/board_paths.h>
#include <stdlib.h>

/*===========================================================================
 * Playback modes
 *===========================================================================*/
typedef enum
{
    PLAY_MODE_SEQUENTIAL = 0,
    PLAY_MODE_SINGLE,
    PLAY_MODE_SHUFFLE,
    PLAY_MODE_COUNT,
} play_mode_t;

static const char * const g_mode_names[PLAY_MODE_COUNT] =
{
    "Mode: Sequential",
    "Mode: Repeat One",
    "Mode: Shuffle",
};

/*===========================================================================
 * Playlist data
 *===========================================================================*/
typedef struct
{
    char filename[64];
} playlist_entry_t;

static playlist_entry_t *g_playlist = NULL;
static int               g_playlist_len = 0;
static int               g_current_idx = 0;

/* true when user has pressed play and expects auto-advance */
static bool              g_player_active = false;

static play_mode_t       g_play_mode = PLAY_MODE_SEQUENTIAL;

/*===========================================================================
 * LVGL widget pointers
 *===========================================================================*/
lv_obj_t *ui_MusicPlayer = NULL;

static lv_obj_t *g_label_song     = NULL;
static lv_obj_t *g_label_progress = NULL;
static lv_obj_t *g_label_nosong   = NULL;
static lv_obj_t *g_btn_mode       = NULL;
static lv_obj_t *g_btn_prev       = NULL;
static lv_obj_t *g_btn_play       = NULL;
static lv_obj_t *g_btn_next       = NULL;
static lv_obj_t *g_label_play     = NULL;
static lv_obj_t *g_label_mode     = NULL;
static lv_timer_t *g_check_timer  = NULL;

/* Currently loaded song (on-demand) */
static buzzer_note_t *g_current_notes = NULL;
static size_t         g_current_notes_count = 0;

/*===========================================================================
 * Forward declarations
 *===========================================================================*/
static void free_playlist(void);
static int  scan_music_files(void);
static void update_song_display(void);
static void play_current_song(void);

/*===========================================================================
 * Playlist navigation helpers
 *===========================================================================*/

static int get_next_index(int current)
{
    switch (g_play_mode)
    {
    case PLAY_MODE_SINGLE:
        return current;

    case PLAY_MODE_SEQUENTIAL:
        return (current + 1) % g_playlist_len;

    case PLAY_MODE_SHUFFLE:
        if (g_playlist_len <= 1)
            return 0;
        int next;
        do {
            next = rand() % g_playlist_len;
        } while (next == current);
        return next;
    }
    return 0;
}

static int get_prev_index(int current)
{
    return (current - 1 + g_playlist_len) % g_playlist_len;
}

/*===========================================================================
 * Playback
 *===========================================================================*/

static void play_current_song(void)
{
    if (g_playlist_len == 0)
        return;
    if (g_current_idx < 0 || g_current_idx >= g_playlist_len)
        return;

    /* Load current song from SD card */
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", SD_MUSIC_DIR,
             g_playlist[g_current_idx].filename);

    size_t count = 0;
    buzzer_note_t *notes = buzzer_load_music(path, &count);
    if (!notes || count == 0)
    {
        printf("music_player: failed to load '%s'\n",
               g_playlist[g_current_idx].filename);
        if (notes)
            buzzer_free_music(notes);
        return;
    }

    /* Stop old playback and free previous notes */
    buzzer_stop_music();
    if (g_current_notes)
        buzzer_free_music(g_current_notes);
    g_current_notes = notes;
    g_current_notes_count = count;

    buzzer_play_music(g_current_notes, g_current_notes_count);
    g_player_active = true;
    lv_label_set_text(g_label_play, "[]");
}

/*===========================================================================
 * Timer: detect natural song end → auto-advance
 *===========================================================================*/

static void check_playback_cb(lv_timer_t *t)
{
    if (g_playlist_len == 0 || !g_player_active)
        return;

    if (!buzzer_is_playing())
    {
        /* Song ended naturally (player active + not playing) */
        if (g_play_mode == PLAY_MODE_SINGLE)
        {
            play_current_song();
        }
        else
        {
            g_current_idx = get_next_index(g_current_idx);
            update_song_display();
            play_current_song();
        }
    }
}

/*===========================================================================
 * Callbacks
 *===========================================================================*/

static void home_click_cb(lv_event_t *e)
{
    _ui_screen_change(&ui_HomeScreen, LV_SCR_LOAD_ANIM_NONE,
                      0, 0, &ui_HomeScreen_screen_init);
}

static void play_stop_cb(lv_event_t *e)
{
    if (g_playlist_len == 0)
        return;

    if (buzzer_is_playing())
    {
        buzzer_stop_music();
        g_player_active = false;
        lv_label_set_text(g_label_play, ">");  /* play */
    }
    else
    {
        play_current_song();
    }
}

static void next_cb(lv_event_t *e)
{
    if (g_playlist_len == 0)
        return;

    buzzer_stop_music();
    g_current_idx = (g_current_idx + 1) % g_playlist_len;
    update_song_display();

    if (g_player_active)
        play_current_song();
}

static void prev_cb(lv_event_t *e)
{
    if (g_playlist_len == 0)
        return;

    buzzer_stop_music();
    g_current_idx = get_prev_index(g_current_idx);
    update_song_display();

    if (g_player_active)
        play_current_song();
}

static void mode_cb(lv_event_t *e)
{
    g_play_mode = (play_mode_t)((g_play_mode + 1) % PLAY_MODE_COUNT);
    lv_label_set_text(g_label_mode, g_mode_names[g_play_mode]);
}

/*===========================================================================
 * Display helpers
 *===========================================================================*/

static void update_song_display(void)
{
    if (g_playlist_len == 0)
    {
        lv_label_set_text(g_label_song, "---");
        lv_label_set_text(g_label_progress, "");
        return;
    }

    lv_label_set_text(g_label_song, g_playlist[g_current_idx].filename);

    char buf[32];
    snprintf(buf, sizeof(buf), "%d / %d", g_current_idx + 1, g_playlist_len);
    lv_label_set_text(g_label_progress, buf);
}

/*===========================================================================
 * Scan SD card for melody files
 *===========================================================================*/

static int scan_music_files(void)
{
    DIR *dir = opendir(SD_MUSIC_DIR);
    if (!dir)
        return 0;

    struct dirent *entry;
    struct stat    statbuf;
    char           path[256];

    while ((entry = readdir(dir)) != NULL)
    {
        /* Only accept .txt files */
        const char *dot = strrchr(entry->d_name, '.');
        if (!dot || strcasecmp(dot, ".txt") != 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", SD_MUSIC_DIR, entry->d_name);
        if (stat(path, &statbuf) < 0 || !S_ISREG(statbuf.st_mode))
            continue;

        playlist_entry_t *p = realloc(g_playlist,
                                      (g_playlist_len + 1) *
                                      sizeof(playlist_entry_t));
        if (!p)
            continue;
        g_playlist = p;

        strncpy(g_playlist[g_playlist_len].filename, entry->d_name,
                sizeof(g_playlist[g_playlist_len].filename) - 1);
        g_playlist[g_playlist_len].filename[
            sizeof(g_playlist[g_playlist_len].filename) - 1] = '\0';
        g_playlist_len++;
    }

    closedir(dir);
    return g_playlist_len;
}

static void free_playlist(void)
{
    if (g_current_notes)
    {
        buzzer_free_music(g_current_notes);
        g_current_notes = NULL;
        g_current_notes_count = 0;
    }
    free(g_playlist);
    g_playlist     = NULL;
    g_playlist_len = 0;
    g_current_idx  = 0;
}

/*===========================================================================
 * Screen init / destroy
 *===========================================================================*/

void ui_MusicPlayer_screen_init(void)
{
    ui_MusicPlayer = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_MusicPlayer, LV_OBJ_FLAG_SCROLLABLE);

    /* ---- Home button (top-left) ---- */
    lv_obj_t *btn_home = lv_button_create(ui_MusicPlayer);
    lv_obj_set_size(btn_home, 90, 40);
    lv_obj_align(btn_home, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(btn_home, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_set_style_radius(btn_home, 8, LV_PART_MAIN);
    lv_obj_t *lbl_home = lv_label_create(btn_home);
    lv_label_set_text(lbl_home, "Home");
    lv_obj_center(lbl_home);
    lv_obj_add_event_cb(btn_home, home_click_cb, LV_EVENT_CLICKED, NULL);

    /* ---- Title ---- */
    lv_obj_t *title = lv_label_create(ui_MusicPlayer);
    lv_label_set_text(title, "Music Player");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14,
                               LV_PART_MAIN);

    /* ---- Song name panel ---- */
    lv_obj_t *panel = lv_obj_create(ui_MusicPlayer);
    lv_obj_set_size(panel, 360, 90);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, -90);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xCCCCCC),
                                  LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 8, LV_PART_MAIN);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    g_label_song = lv_label_create(panel);
    lv_obj_align(g_label_song, LV_ALIGN_CENTER, 0, -14);
    lv_label_set_text(g_label_song, "---");

    g_label_progress = lv_label_create(panel);
    lv_obj_align(g_label_progress, LV_ALIGN_CENTER, 0, 14);
    lv_obj_set_style_text_color(g_label_progress, lv_color_hex(0x888888),
                                LV_PART_MAIN);

    /* ---- No-song message (hidden if songs exist) ---- */
    g_label_nosong = lv_label_create(ui_MusicPlayer);
    lv_label_set_text(g_label_nosong, "no song");
    lv_obj_align(g_label_nosong, LV_ALIGN_CENTER, 0, -90);
    lv_obj_set_style_text_font(g_label_nosong, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_text_color(g_label_nosong, lv_color_hex(0xE05050),
                                LV_PART_MAIN);

    /* ---- Mode button ---- */
    g_btn_mode = lv_button_create(ui_MusicPlayer);
    lv_obj_set_size(g_btn_mode, 140, 36);
    lv_obj_align(g_btn_mode, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(g_btn_mode, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_set_style_radius(g_btn_mode, 6, LV_PART_MAIN);
    g_label_mode = lv_label_create(g_btn_mode);
    lv_label_set_text(g_label_mode, g_mode_names[g_play_mode]);
    lv_obj_center(g_label_mode);
    lv_obj_add_event_cb(g_btn_mode, mode_cb, LV_EVENT_CLICKED, NULL);

    /* ---- Transport controls ---- */
    const int btn_size = 52;
    const int gap      = 30;

    /* Previous */
    g_btn_prev = lv_button_create(ui_MusicPlayer);
    lv_obj_set_size(g_btn_prev, btn_size, btn_size);
    lv_obj_align(g_btn_prev, LV_ALIGN_CENTER, -(btn_size + gap), 50);
    lv_obj_set_style_radius(g_btn_prev, 26, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_btn_prev, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_t *lbl_prev = lv_label_create(g_btn_prev);
    lv_label_set_text(lbl_prev, "<<");
    lv_obj_center(lbl_prev);
    lv_obj_add_event_cb(g_btn_prev, prev_cb, LV_EVENT_CLICKED, NULL);

    /* Play / Stop */
    g_btn_play = lv_button_create(ui_MusicPlayer);
    lv_obj_set_size(g_btn_play, btn_size, btn_size);
    lv_obj_align(g_btn_play, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_radius(g_btn_play, 26, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_btn_play, lv_color_hex(0x2E7D32),
                              LV_PART_MAIN);
    g_label_play = lv_label_create(g_btn_play);
    lv_label_set_text(g_label_play, ">");
    lv_obj_center(g_label_play);
    lv_obj_add_event_cb(g_btn_play, play_stop_cb, LV_EVENT_CLICKED, NULL);

    /* Next */
    g_btn_next = lv_button_create(ui_MusicPlayer);
    lv_obj_set_size(g_btn_next, btn_size, btn_size);
    lv_obj_align(g_btn_next, LV_ALIGN_CENTER, (btn_size + gap), 50);
    lv_obj_set_style_radius(g_btn_next, 26, LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_btn_next, lv_color_hex(0x3AACB7),
                              LV_PART_MAIN);
    lv_obj_t *lbl_next = lv_label_create(g_btn_next);
    lv_label_set_text(lbl_next, ">>");
    lv_obj_center(lbl_next);
    lv_obj_add_event_cb(g_btn_next, next_cb, LV_EVENT_CLICKED, NULL);

    /* ---- Terminal toggle (bottom-right) ---- */
    nsh_terminal_toggle_btn_create(ui_MusicPlayer);

    /* ---- Seed shuffle RNG ---- */
    srand(1);

    /* ---- Scan playlist ---- */
    free_playlist();
    scan_music_files();

    if (g_playlist_len == 0)
    {
        lv_obj_clear_flag(g_label_nosong, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_btn_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_btn_prev, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_btn_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_btn_next, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(g_label_nosong, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_btn_mode, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_btn_prev, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_btn_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_btn_next, LV_OBJ_FLAG_HIDDEN);
        update_song_display();
    }

    /* ---- Playback check timer (every 200 ms) ---- */
    g_check_timer = lv_timer_create(check_playback_cb, 200, NULL);
}

void ui_MusicPlayer_screen_destroy(void)
{
    buzzer_stop_music();

    if (g_check_timer)
    {
        lv_timer_del(g_check_timer);
        g_check_timer = NULL;
    }

    free_playlist();

    if (ui_MusicPlayer)
        lv_obj_del(ui_MusicPlayer);

    ui_MusicPlayer   = NULL;
    g_label_song     = NULL;
    g_label_progress = NULL;
    g_label_nosong   = NULL;
    g_btn_mode       = NULL;
    g_btn_prev       = NULL;
    g_btn_play       = NULL;
    g_btn_next       = NULL;
    g_label_play     = NULL;
    g_label_mode     = NULL;
}
