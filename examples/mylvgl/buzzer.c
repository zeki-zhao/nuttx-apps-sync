#include <nuttx/config.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "buzzer.h"

/* Note names for piano UI */
static const char * const g_note_names[NOTE_COUNT] = {
    "Do", "Re", "Mi", "Fa", "Sol", "La", "Si"
};

/* Half-period lookup for piano UI (maps enum to value, C5 octave @1MHz) */
static const int g_half_period_us[NOTE_COUNT] = {
    955,  /* Do   523.25 Hz */
    850,  /* Re   587.33 Hz */
    757,  /* Mi   659.25 Hz */
    715,  /* Fa   698.46 Hz */
    637,  /* Sol  783.99 Hz */
    567,  /* La   880.00 Hz */
    505,  /* Si   987.77 Hz */
};

/* Music player state */
static volatile bool       g_music_playing = false;
static pthread_t           g_music_thread;
static const buzzer_note_t *g_music_seq = NULL;
static size_t              g_music_len = 0;

/* Nokia boot melody */
static const buzzer_note_t g_nokia_ringtone[] =
{
    {757, 180},  /* Mi   E5 */
    {850, 180},  /* Re   D5 */
    {675, 180},  /* Fa#  F#5 */
    {601, 200},  /* Sol# G#5 */
    {450, 400},  /* Do#  C#6 */
};
#define NOKIA_COUNT (sizeof(g_nokia_ringtone) / sizeof(g_nokia_ringtone[0]))

extern void board_buzzer_start(int half_period_us);
extern void board_buzzer_stop(void);
extern void board_buzzer_set_period(int half_period_us);
extern int  board_note_to_hp(const char *name);

void buzzer_init(void)
{
}

void buzzer_deinit(void)
{
    buzzer_stop_music();
    buzzer_off(NOTE_DO);
}

void buzzer_on(note_name_t note)
{
    if (note >= NOTE_COUNT)
        return;

    /* Stop music playback on manual key press */
    g_music_playing = false;
    board_buzzer_start(g_half_period_us[note]);

    printf("buzzer: %s on (%d us)\n", g_note_names[note],
           g_half_period_us[note]);
}

void buzzer_off(note_name_t note)
{
    board_buzzer_stop();

    printf("buzzer: %s off\n", g_note_names[note]);
}

/* === Music player thread === */

static void *music_player_fn(void *arg)
{
    (void)arg;

    bool was_stopped = true;

    for (size_t i = 0; i < g_music_len && g_music_playing; i++)
    {
        const buzzer_note_t *n = &g_music_seq[i];

        if (n->half_period_us > 0)
        {
            if (was_stopped)
            {
                board_buzzer_start(n->half_period_us);
                was_stopped = false;
            }
            else
            {
                board_buzzer_set_period(n->half_period_us);
            }
        }
        else
        {
            board_buzzer_stop();
            was_stopped = true;
        }

        /* Chunked sleep for responsive stop */
        int remaining = n->duration_ms;
        while (remaining > 0 && g_music_playing)
        {
            int chunk = remaining > 50 ? 50 : remaining;
            usleep(chunk * 1000);
            remaining -= chunk;
        }
    }

    board_buzzer_stop();
    g_music_playing = false;
    return NULL;
}

void buzzer_play_music(const buzzer_note_t *notes, size_t count)
{
    buzzer_stop_music();

    g_music_seq = notes;
    g_music_len = count;
    g_music_playing = true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
    pthread_create(&g_music_thread, &attr, music_player_fn, NULL);
    pthread_detach(g_music_thread);
    pthread_attr_destroy(&attr);
}

void buzzer_stop_music(void)
{
    g_music_playing = false;
    board_buzzer_stop();
}

bool buzzer_is_playing(void)
{
    return g_music_playing;
}

void buzzer_play_nokia_ringtone(void)
{
    buzzer_play_music(g_nokia_ringtone, NOKIA_COUNT);
}

/*==========================================================================
 * Music file loader (text format for FAT filesystem)
 *   Lines: "Note Duration_ms"  e.g. "E5 180"
 *          "R 200" for rest
 *          "# comment"
 *==========================================================================*/

buzzer_note_t *buzzer_load_music(const char *filepath, size_t *count)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
    {
        printf("buzzer: can't open %s\n", filepath);
        return NULL;
    }

    /* First pass: count valid note lines */
    size_t n = 0;
    char line[64];
    while (fgets(line, sizeof(line), f))
    {
        const char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\0')
            continue;

        char note[16];
        int dur;
        if (sscanf(s, "%15s %d", note, &dur) >= 2)
            n++;
    }

    if (n == 0)
    {
        fclose(f);
        printf("buzzer: no notes in %s\n", filepath);
        return NULL;
    }

    buzzer_note_t *notes = malloc(n * sizeof(buzzer_note_t));
    if (!notes)
    {
        fclose(f);
        return NULL;
    }

    /* Second pass: parse each line */
    rewind(f);
    size_t i = 0;
    while (i < n && fgets(line, sizeof(line), f))
    {
        const char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '#' || *s == '\n' || *s == '\0')
            continue;

        char note[16];
        int dur;
        if (sscanf(s, "%15s %d", note, &dur) < 2)
            continue;

        int hp = board_note_to_hp(note);
        if (hp < 0)
        {
            printf("buzzer: skip unknown note '%s'\n", note);
            continue;
        }
        notes[i].half_period_us = (uint16_t)hp;
        notes[i].duration_ms = (uint16_t)dur;
        i++;
    }

    fclose(f);
    *count = i;
    return notes;
}

void buzzer_free_music(buzzer_note_t *notes)
{
    free(notes);
}
