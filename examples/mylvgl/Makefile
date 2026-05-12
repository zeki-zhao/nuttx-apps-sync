############################################################################
# apps/examples/mylvgl/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

PROGNAME = mylvgl
PRIORITY = $(CONFIG_EXAMPLES_MY_LVGLPRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_MY_LVGL_STACKSIZE)
MODULE = $(CONFIG_EXAMPLES_MY_LVGL)

# LVGL mylvgl Example

MAINSRC = mylvgl.c

CSRCS += modbus_data.c
CSRCS += ui/ui.c
CSRCS += ui/ui_helpers.c
CSRCS += ui/ui_events.c
CSRCS += ui/screens/ui_HomeScreen.c
CSRCS += ui/screens/ui_Screen1.c
CSRCS += ui/components/ui_comp.c
CSRCS += ui/components/ui_comp_changeled.c
CSRCS += ui/components/ui_comp_hook.c

CFLAGS += -I$(APPDIR)/examples/mylvgl
CFLAGS += -I$(APPDIR)/examples/mylvgl/ui

include $(APPDIR)/Application.mk
