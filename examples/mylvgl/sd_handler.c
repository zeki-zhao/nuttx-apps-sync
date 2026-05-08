#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "sd_handler.h"

#define SD_SAVE_DIR  "/mnt/sd/TEST"
#define SD_SAVE_FILE  "lvgl_input.txt"

void save_text_handler(const struct lvgl_msg_s *msg)
{
    if (!msg->text)
        return;

    /* create TEST directory if it doesn't exist */
    mkdir(SD_SAVE_DIR, 0777);

    /* generate unique filename with a counter */
    char path[128];
    snprintf(path, sizeof(path), SD_SAVE_DIR "/" SD_SAVE_FILE);

    FILE *fp = fopen(path, "w");
    if (fp)
    {
        fwrite(msg->text, 1, strlen(msg->text), fp);
        fclose(fp);
        printf("Saved: %s\n", path);
    }
    else
    {
        printf("ERROR: Failed to write %s: %d\n", path, errno);
    }

    free(msg->text);
}
