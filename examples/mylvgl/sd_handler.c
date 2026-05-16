#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "netutils/cJSON.h"
#include "sd_handler.h"
#include "lvgl_event.h"
#include <arch/board/board_paths.h>

#define SD_SAVE_DIR  SD_LOG_DIR
#define SD_SAVE_FILE  "lvgl_input.txt"

#define FILE_C_NAME  "lvgl_asd.c"

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
    if (fp){
        fwrite(msg->text, 1, strlen(msg->text), fp);
        fclose(fp);
        printf("Saved: %s\n", path);
    }else{
        printf("ERROR: Failed to write %s: %d\n", path, errno);
    }

    free(msg->text);

    // cJSON *root;
    // char *out;
    // static const char *strings[7] =
    // {
    //     "Sunday", "Monday", "Tuesday", "Wednesday",
    //     "Thursday", "Friday", "Saturday"
    // };
    // root = cJSON_CreateStringArray(strings, 7);
    // out = cJSON_Print(root);
    // cJSON_Delete(root);
    // printf("%s\n", out);
    // free(out);


    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), SD_SAVE_DIR "/" FILE_C_NAME);
    FILE *fp2 = fopen(path, "w");
    if (fp2) {
        fwrite("hello\n", 1, strlen("hello\n"), fp2);
        fclose(fp2);
        printf("Created: %s\n", path);
    }else{
        printf("ERROR: Failed to write %s: %d\n", path, errno);
    }

}

int lvgl_event_send_text(const char *text)
{
    mqd_t lvgl_mqd = lvgl_event_get_mqd(LVGL_EVENT_MQ_NAME);
    if (lvgl_mqd == (mqd_t)-1 || !text)
        return -ENODEV;

    struct lvgl_msg_s msg;
    msg.type = LVGL_MSG_SAVE_TEXT;
    msg.text = strdup(text);
    if (!msg.text)
        return -ENOMEM;

    int ret = mq_send(lvgl_mqd, (const char *)&msg, sizeof(msg), 0);
    if (ret < 0)
    {
        free(msg.text);
        return ret;
    }
    return OK;
}




