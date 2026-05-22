/****************************************************************************
 * apps/examples/mylvgl/nsh_terminal.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <poll.h>
#include <spawn.h>
#include <errno.h>
#include <lvgl/lvgl.h>

#include "nsh_terminal.h"
#include "screens/ui_Terminal.h"
#include "screens/ui_HomeScreen.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TIMER_PERIOD_MS  100
#define READ_PIPE         0
#define WRITE_PIPE        1
#define NSH_TASK         "nsh"

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Pipes for NSH Shell: stdin, stdout, stderr */

static int g_nsh_stdin[2];
static int g_nsh_stdout[2];
static int g_nsh_stderr[2];

/* Timer */

static lv_timer_t   *g_term_timer;

/* Arguments for NSH Task */

static FAR char * const g_nsh_argv[] =
{
  NSH_TASK, NULL
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: has_input
 *
 * Description:
 *   Return true if the file descriptor has data to be read.
 *
 ****************************************************************************/

static bool has_input(int fd)
{
  int ret;
  struct pollfd fdp;

  fdp.fd      = fd;
  fdp.events  = POLLIN;
  ret         = poll(&fdp, 1, 0);

  if (ret > 0)
    {
      return (fdp.revents & POLLIN) != 0;
    }

  return false;
}

/****************************************************************************
 * Name: remove_escape_codes
 *
 * Description:
 *   Remove ANSI Escape Codes from the string.
 *
 ****************************************************************************/

static void remove_escape_codes(FAR char *buf, int len)
{
  int i;
  int j;

  for (i = 0; i < len; i++)
    {
      if (buf[i] == 0x1b)
        {
          for (j = i; j + 2 < len; j++)
            {
              buf[j] = buf[j + 3];
            }
        }
    }
}

/****************************************************************************
 * Name: term_timer_callback
 *
 * Description:
 *   LVGL timer callback.  Poll NSH stdout and stderr for output and
 *   display the output in the terminal textarea.
 *
 ****************************************************************************/

static void term_timer_callback(lv_timer_t *timer)
{
  int ret;
  static char buf[64];

  if (has_input(g_nsh_stdout[READ_PIPE]))
    {
      ret = read(g_nsh_stdout[READ_PIPE], buf, sizeof(buf) - 1);
      if (ret > 0)
        {
          buf[ret] = 0;
          remove_escape_codes(buf, ret);
          lv_textarea_add_text(ui_TerminalOutput, buf);
        }
    }

  if (has_input(g_nsh_stderr[READ_PIPE]))
    {
      ret = read(g_nsh_stderr[READ_PIPE], buf, sizeof(buf) - 1);
      if (ret > 0)
        {
          buf[ret] = 0;
          remove_escape_codes(buf, ret);
          lv_textarea_add_text(ui_TerminalOutput, buf);
        }
    }
}

/****************************************************************************
 * Name: term_input_callback
 *
 * Description:
 *   Callback for NSH Input Text Area.  Manages keyboard show/hide and
 *   sends commands to NSH stdin on Enter.
 *
 ****************************************************************************/

static void term_input_callback(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  /* Show keyboard when input is focused */

  if (code == LV_EVENT_FOCUSED)
    {
      lv_obj_clear_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
      return;
    }

  /* Hide keyboard when input loses focus */

  if (code == LV_EVENT_DEFOCUSED)
    {
      lv_obj_add_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
      return;
    }

  if (code == LV_EVENT_VALUE_CHANGED)
    {
      static char prev_input[256];
      const char *input = lv_textarea_get_text(ui_TerminalInput);
      int prev_len = strlen(prev_input);
      int input_len = strlen(input);
      const uint16_t id = lv_keyboard_get_selected_button(ui_TerminalKeyboard);
      const char *key = lv_keyboard_get_button_text(ui_TerminalKeyboard, id);

      if (key == NULL)
        {
          strcpy(prev_input, input);
          return;
        }

      /* Enter key pressed — send command to NSH stdin */

      if (key[0] == 0xef && key[1] == 0xa2 && key[2] == 0xa2)
        {
          const char *cmd = lv_textarea_get_text(ui_TerminalInput);

          if (cmd == NULL || cmd[0] == 0)
            {
              return;
            }

          write(g_nsh_stdin[WRITE_PIPE], cmd, strlen(cmd));
          lv_textarea_add_text(ui_TerminalOutput, "\n");
          prev_input[0] = '\0';
          lv_textarea_set_text(ui_TerminalInput, "");
          lv_obj_add_flag(ui_TerminalKeyboard, LV_OBJ_FLAG_HIDDEN);
          return;
        }

      /* Echo input characters to output */

      if (input_len > prev_len)
        {
          lv_textarea_add_text(ui_TerminalOutput, input + prev_len);
          strcpy(prev_input, input);
        }
      else if (input_len < prev_len)
        {
          lv_textarea_set_cursor_pos(ui_TerminalOutput,
                                     LV_TEXTAREA_CURSOR_LAST);
          lv_textarea_delete_char(ui_TerminalOutput);
          strcpy(prev_input, input);
        }
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nsh_terminal_init
 *
 * Description:
 *   Initialize the NSH terminal: create pipes, spawn NSH, create terminal
 *   screen (hidden), and create polling timer.
 *
 ****************************************************************************/

int nsh_terminal_init(void)
{
  int ret;
  pid_t pid;

  /* Create pipes for NSH stdin, stdout, and stderr */

  ret = pipe(g_nsh_stdin);
  if (ret < 0)
    {
      _err("stdin pipe failed: %d\n", errno);
      return ERROR;
    }

  ret = pipe(g_nsh_stdout);
  if (ret < 0)
    {
      _err("stdout pipe failed: %d\n", errno);
      return ERROR;
    }

  ret = pipe(g_nsh_stderr);
  if (ret < 0)
    {
      _err("stderr pipe failed: %d\n", errno);
      return ERROR;
    }

  /* Spawn NSH with redirected stdin/stdout/stderr via file_actions.
   * This avoids modifying the parent's file descriptors.
   */

  {
    posix_spawn_file_actions_t actions;

    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions,
                                     g_nsh_stdin[READ_PIPE], 0);
    posix_spawn_file_actions_adddup2(&actions,
                                     g_nsh_stdout[WRITE_PIPE], 1);
    posix_spawn_file_actions_adddup2(&actions,
                                     g_nsh_stderr[WRITE_PIPE], 2);

    ret = posix_spawn(&pid, NSH_TASK, &actions, NULL,
                      g_nsh_argv, NULL);

    posix_spawn_file_actions_destroy(&actions);
  }

  if (ret != 0)
    {
      _err("posix_spawn nsh failed: %d\n", ret);
      return -ret;
    }

  /* Register input callback on terminal input textarea */

  lv_obj_add_event_cb(ui_TerminalInput, term_input_callback,
                      LV_EVENT_ALL, NULL);

  /* Create LVGL timer to poll NSH output */

  g_term_timer = lv_timer_create(term_timer_callback,
                                 TIMER_PERIOD_MS, NULL);

  return OK;
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
      return ERROR;
    }

  lv_scr_load(ui_Terminal);
  return OK;
}

/****************************************************************************
 * Name: nsh_terminal_close
 *
 * Description:
 *   Switch back to the HomeScreen.
 *
 ****************************************************************************/

int nsh_terminal_close(void)
{
  lv_scr_load(ui_HomeScreen);
  return OK;
}
