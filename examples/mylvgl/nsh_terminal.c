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
#include <fcntl.h>
#include <errno.h>
#include <lvgl/lvgl.h>

#include <nuttx/usb/usbhost.h>

#include "nsh_terminal.h"
#include "screens/ui_Terminal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define TIMER_PERIOD_MS     100
#define READ_PIPE            0
#define WRITE_PIPE           1
#define NSH_TASK            "nsh"

/* Default keyboard device path */

#ifndef CONFIG_EXAMPLES_MY_LVGL_KBD_DEVPATH
#  define CONFIG_EXAMPLES_MY_LVGL_KBD_DEVPATH "/dev/kbda"
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Pipes for NSH Shell: stdin, stdout, stderr */

static int g_nsh_stdin[2];
static int g_nsh_stdout[2];
static int g_nsh_stderr[2];

/* Timer */

static lv_timer_t   *g_term_timer;

/* Keyboard device */

static int           g_kbd_fd = -1;

/* Shared line buffer for physical + virtual keyboard input */

static char          g_kbd_line[256];
static int           g_kbd_line_len;

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

static int has_input(int fd)
{
  int ret;
  struct pollfd fdp;

  fdp.fd      = fd;
  fdp.events  = POLLIN;
  ret         = poll(&fdp, 1, 0);

  if (ret > 0)
    {
      if (fdp.revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL))
        {
          return 1;
        }
    }

  return 0;
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
 *   LVGL timer callback.  Poll NSH stdout, stderr for output and display
 *   in the terminal; poll keyboard device and feed input to NSH stdin.
 *
 ****************************************************************************/

static void term_timer_callback(lv_timer_t *timer)
{
  int ret;
  static char buf[64];
  bool new_output = false;
  bool active = (lv_scr_act() == ui_Terminal);

  if (has_input(g_nsh_stdout[READ_PIPE]))
    {
      ret = read(g_nsh_stdout[READ_PIPE], buf, sizeof(buf) - 1);
      if (ret > 0)
        {
          buf[ret] = 0;
          remove_escape_codes(buf, ret);
          term_label_append(ui_TerminalOutput, buf);
          new_output = true;
        }
    }

  if (has_input(g_nsh_stderr[READ_PIPE]))
    {
      ret = read(g_nsh_stderr[READ_PIPE], buf, sizeof(buf) - 1);
      if (ret > 0)
        {
          buf[ret] = 0;
          remove_escape_codes(buf, ret);
          term_label_append(ui_TerminalOutput, buf);
          new_output = true;
        }
    }

  /* Trim output to prevent unbounded growth / redraw slowdown */

  trim_textarea(ui_TerminalOutput);

  /* UI updates (scroll + keyboard) only when terminal is the active screen */

  if (active)
    {
      /* Only scroll to bottom when new output was actually added,
       * otherwise the user's manual scroll-up would be overridden. */

      if (new_output)
        {
          term_scroll_bottom(ui_TerminalOutput);
        }

      /* Retry opening keyboard device if not yet available */

      if (g_kbd_fd < 0)
        {
          g_kbd_fd = open(CONFIG_EXAMPLES_MY_LVGL_KBD_DEVPATH, O_RDONLY);
          if (g_kbd_fd >= 0)
            {
              _warn("Keyboard device %s opened (deferred)\n",
                    CONFIG_EXAMPLES_MY_LVGL_KBD_DEVPATH);
            }
        }

      /* Poll keyboard device and feed input to NSH stdin */

      if (g_kbd_fd >= 0 && has_input(g_kbd_fd))
        {
          ret = read(g_kbd_fd, buf, sizeof(buf) - 1);
          if (ret <= 0)
            {
              close(g_kbd_fd);
              g_kbd_fd = -1;
            }
          else
            {
              int i;

              for (i = 0; i < ret; i++)
                {
                  char ch = buf[i];

                  if (ch == '\r' || ch == '\n')
                    {
                      /* Send buffered line to NSH stdin */

                      if (g_kbd_line_len > 0)
                        {
                          write(g_nsh_stdin[WRITE_PIPE], g_kbd_line,
                                g_kbd_line_len);
                        }

                      write(g_nsh_stdin[WRITE_PIPE], "\n", 1);
                      term_label_append(ui_TerminalOutput, "\n");
                      g_kbd_line_len = 0;
                    }
                  else if (ch == 0x7f || ch == 0x08)
                    {
                      /* Backspace */

                      if (g_kbd_line_len > 0)
                        {
                          g_kbd_line_len--;
                          term_backspace(ui_TerminalOutput);
                        }
                    }
                  else if (ch >= 0x20 && ch <= 0x7e)
                    {
                      /* Printable character */

                      if (g_kbd_line_len < (int)sizeof(g_kbd_line) - 1)
                        {
                          g_kbd_line[g_kbd_line_len++] = ch;
                        }

                      term_label_append(ui_TerminalOutput,
                                           (const char[]){ch, '\0'});
                    }
                }

              term_scroll_bottom(ui_TerminalOutput);
            }
        }
    }
}

/****************************************************************************
 * Name: term_kbd_input_cb
 *
 * Description:
 *   Handle virtual keyboard key presses.  Shares the line buffer with
 *   the physical keyboard path.
 *
 ****************************************************************************/

static void term_kbd_input_cb(lv_event_t *e)
{
  const uint16_t id = lv_keyboard_get_selected_button(ui_TerminalKeyboard);
  const char *key = lv_keyboard_get_button_text(ui_TerminalKeyboard, id);
  size_t len;

  if (key == NULL || key[0] == '\0')
    {
      return;
    }

  len = strlen(key);

  /* Enter key (UTF-8: 0xEF 0xA2 0xA2) */

  if (len == 3 && (uint8_t)key[0] == 0xef &&
      (uint8_t)key[1] == 0xa2 && (uint8_t)key[2] == 0xa2)
    {
      if (g_kbd_line_len > 0)
        {
          write(g_nsh_stdin[WRITE_PIPE], g_kbd_line, g_kbd_line_len);
        }

      write(g_nsh_stdin[WRITE_PIPE], "\n", 1);
      term_label_append(ui_TerminalOutput, "\n");
      g_kbd_line_len = 0;
      term_scroll_bottom(ui_TerminalOutput);
      return;
    }

  /* Backspace key: standard LV_SYMBOL_BACKSPACE (0xEF 0x80 0x8B)
   * or the board-specific key (0xEF 0x95 0x9A) */

  if (len == 3 && (uint8_t)key[0] == 0xef &&
      (((uint8_t)key[1] == 0x80 && (uint8_t)key[2] == 0x8b) ||
       ((uint8_t)key[1] == 0x95 && (uint8_t)key[2] == 0x9a)))
    {
      if (g_kbd_line_len > 0)
        {
          g_kbd_line_len--;
          term_backspace(ui_TerminalOutput);
        }

      return;
    }

  /* Regular character (single-byte ASCII) */

  if (len == 1 && key[0] >= 0x20 && key[0] <= 0x7e)
    {
      if (g_kbd_line_len < (int)sizeof(g_kbd_line) - 1)
        {
          g_kbd_line[g_kbd_line_len++] = key[0];
        }

      term_label_append(ui_TerminalOutput, key);
      term_scroll_bottom(ui_TerminalOutput);
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

  /* Create LVGL timer to poll NSH output */

  g_term_timer = lv_timer_create(term_timer_callback,
                                 TIMER_PERIOD_MS, NULL);

  /* Remove the stand-alone input textarea; all input goes directly
   * to the output area.
   */

  lv_obj_del(ui_TerminalInput);
  ui_TerminalInput = NULL;

  /* Clear default "Text" that lv_label_create puts */

  lv_label_set_text(ui_TerminalOutput, "");

  /* Expand output to full screen; it will shrink when keyboard is opened */

  lv_obj_set_height(ui_TerminalOutput, LV_PCT(100));

  /* Detach keyboard from the deleted textarea */

  lv_keyboard_set_textarea(ui_TerminalKeyboard, NULL);

  /* Make output area clickable to toggle virtual keyboard */

  lv_obj_add_flag(ui_TerminalOutput, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(ui_TerminalOutput, term_output_click_cb,
                      LV_EVENT_CLICKED, NULL);

  /* Register callback for virtual keyboard key presses */

  lv_obj_add_event_cb(ui_TerminalKeyboard, term_kbd_input_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  return OK;
}
