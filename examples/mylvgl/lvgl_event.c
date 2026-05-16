#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <mqueue.h>

#include "lvgl_event.h"

static mqd_t g_lvgl_mqd = (mqd_t)-1;

/* registration table */
static struct
{
    long type;
    lvgl_evt_handler_t handler;
} g_handlers[LVGL_EVT_MAX_HANDLERS];
static int g_num_handlers;

int lvgl_evt_register(long type, lvgl_evt_handler_t handler)
{
    if (g_num_handlers >= LVGL_EVT_MAX_HANDLERS)
        return -ENOMEM;
    g_handlers[g_num_handlers].type    = type;
    g_handlers[g_num_handlers].handler = handler;
    g_num_handlers++;
    return OK;
}

void lvgl_evt_unregister(long type)
{
    for (int i = 0; i < g_num_handlers; i++)
    {
        if (g_handlers[i].type == type)
        {
            int n = g_num_handlers - i - 1;
            if (n > 0)
                memmove(&g_handlers[i], &g_handlers[i + 1],
                        n * sizeof(g_handlers[0]));
            g_num_handlers--;
            return;
        }
    }
}

mqd_t lvgl_event_init(void)
{
    struct mq_attr attr =
    {
        .mq_maxmsg  = 8,
        .mq_msgsize = sizeof(struct lvgl_msg_s),
        .mq_flags   = 0,
    };

    g_lvgl_mqd = mq_open(LVGL_EVENT_MQ_NAME,
                          O_CREAT | O_RDWR, 0666, &attr);
    if (g_lvgl_mqd == (mqd_t)-1)
        printf("ERROR: Failed to create mq: %d\n", errno);

    return g_lvgl_mqd;
}

void lvgl_event_fini(void)
{
    if (g_lvgl_mqd != (mqd_t)-1)
    {
        mq_close(g_lvgl_mqd);
        mq_unlink(LVGL_EVENT_MQ_NAME);
        g_lvgl_mqd = (mqd_t)-1;
    }
}


mqd_t lvgl_event_get_mqd(const char *name)
{
    if (name && strcmp(name, LVGL_EVENT_MQ_NAME) == 0)
        return g_lvgl_mqd;
    return (mqd_t)-1;
}

static void handle_event(const struct lvgl_msg_s *msg)
{
    for (int i = 0; i < g_num_handlers; i++)
    {
        if (g_handlers[i].type == msg->type)
        {
            g_handlers[i].handler(msg);
            return;
        }
    }
    printf("WARNING: Unknown event type %ld\n", msg->type);
}

pthread_addr_t LvglEventProcess(pthread_addr_t arg)
{
    mqd_t mqd = (mqd_t)(intptr_t)arg;
    struct lvgl_msg_s msg; 

    while (1)
    {
        ssize_t n = mq_receive(mqd, (char *)&msg, sizeof(msg), NULL);
        if (n > 0)
            handle_event(&msg);
    }

    return NULL;
}
 