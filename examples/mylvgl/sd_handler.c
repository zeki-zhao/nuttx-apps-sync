#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "netutils/cJSON.h"
#include "sd_handler.h"
#include "lvgl_event.h"
#include <arch/board/board_paths.h>

#define SD_SAVE_FILE  "lvgl_input.txt"

void save_text_handler(const struct lvgl_msg_s *msg)
{
    if (!msg->text)
        return;

    /* generate unique filename with a counter */
    char path[128];
    snprintf(path, sizeof(path), SD_LOG_DIR "/" SD_SAVE_FILE);

    FILE *fp = fopen(path, "w");
    if (fp){
        fwrite(msg->text, 1, strlen(msg->text), fp);
        fclose(fp);
        printf("Saved: %s\n", path);
    }else{
        printf("ERROR: Failed to write %s: %d\n", path, errno);
    }

    free(msg->text);
}


void save_modbus_slave_show_config_hander(const struct lvgl_msg_s *msg)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "start_addr", msg->start_addr);
    cJSON_AddNumberToObject(root, "num_rows", msg->num_rows);
    cJSON_AddNumberToObject(root, "reg_type", msg->reg_type);

    char *out = cJSON_Print(root);
    printf("%s\n", out);

    char path[128];
    snprintf(path, sizeof(path), SD_CONFIG_DIR "/modbus_slave_show_config.json");
    FILE *fp = fopen(path, "w");
    if (fp) {
        fwrite(out, 1, strlen(out), fp);
        fclose(fp);
        printf("Saved: %s\n", path);
    } else {
        printf("ERROR: Failed to write %s: %d\n", path, errno);
    }

    cJSON_Delete(root);
    free(out);
}

int lvgl_event_send_modbus_slave_config(int start_addr, int num_rows, int reg_type)
{
    mqd_t lvgl_mqd = lvgl_event_get_mqd(LVGL_EVENT_MQ_NAME);
    if (lvgl_mqd == (mqd_t)-1)
        return -ENODEV;

    struct lvgl_msg_s msg;
    msg.type       = LVGL_MSG_SAVE_MODBUS_SLAVE_CONFIG;
    msg.start_addr = start_addr;
    msg.num_rows   = num_rows;
    msg.reg_type   = reg_type;

    return mq_send(lvgl_mqd, (const char *)&msg, sizeof(msg), 0);
}

char *load_text_from_sd(void)
{
    char path[128];
    snprintf(path, sizeof(path), SD_LOG_DIR "/" SD_SAVE_FILE);

    FILE *fp = fopen(path, "r");
    if (!fp)
        return NULL;

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fsize <= 0)
    {
        fclose(fp);
        return NULL;
    }

    char *buf = malloc(fsize + 1);
    if (!buf)
    {
        fclose(fp);
        return NULL;
    }

    size_t nread = fread(buf, 1, fsize, fp);
    fclose(fp);
    buf[nread] = '\0';

    return buf;
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




