#ifndef BUZZER_H
#define BUZZER_H

#include <stddef.h>

typedef enum {
    NOTE_DO,
    NOTE_RE,
    NOTE_MI,
    NOTE_FA,
    NOTE_SOL,
    NOTE_LA,
    NOTE_SI,
    NOTE_COUNT,
} note_name_t;

void buzzer_init(void);
void buzzer_deinit(void);
void buzzer_on(note_name_t note);
void buzzer_off(note_name_t note);

/* Music playback */
typedef struct {
    uint16_t half_period_us;  /* 0 = rest/silence */
    uint16_t duration_ms;
} buzzer_note_t;

void buzzer_play_music(const buzzer_note_t *notes, size_t count);
void buzzer_stop_music(void);
bool buzzer_is_playing(void);
void buzzer_play_nokia_ringtone(void);

/* Load music from a text file (FAT filesystem)
 * File format: one "note duration_ms" per line
 *   E5 180
 *   D5 180
 *   F#5 180
 *   R 200      (rest/silence)
 * Lines starting with # are comments.
 */
buzzer_note_t *buzzer_load_music(const char *filepath, size_t *count);
void buzzer_free_music(buzzer_note_t *notes);

#endif /* BUZZER_H */
