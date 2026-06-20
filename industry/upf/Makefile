############################################################################
# apps/industry/upf/Makefile
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

# Universal Protocol Framework (UPF)

UPF_DIR = .
UPF_DIR_NAME = upf

-include ./upf/upf.mk

WD := ${shell echo $(CURDIR) | sed -e 's/ /\\ /g'}

CONFIG_INDUSTRY_UPF_URL_BASE ?= "https://gitee.com/zeki_zhao/universal-protocol-framework/repository/archive"

_MAJ        := $(CONFIG_UPF_VERSION_MAJOR)
_MIN        := $(CONFIG_UPF_VERSION_MINOR)
_PAT        := $(CONFIG_UPF_VERSION_PATCH)
UPF_VERSION := $(_MAJ).$(_MIN).$(_PAT)
UPF_TARBALL := release-v$(UPF_VERSION).zip

UPF_UNPACKNAME = upf
UNPACK ?= unzip -o $(if $(V),,-q)
CURL ?= curl -L -O $(if $(V),,-Ss)

UPF_UNPACKDIR = $(WD)/$(UPF_UNPACKNAME)

$(UPF_TARBALL):
	$(ECHO_BEGIN)"Downloading: $(UPF_TARBALL)"
	$(Q) $(CURL) $(CONFIG_INDUSTRY_UPF_URL_BASE)/$(UPF_TARBALL)
	$(ECHO_END)

$(UPF_UNPACKNAME): $(UPF_TARBALL)
	$(ECHO_BEGIN)"Unpacking: $(UPF_TARBALL) -> $(UPF_UNPACKNAME)"
	$(Q) $(UNPACK) $(UPF_TARBALL)
	$(Q) mv universal-protocol-framework $(UPF_UNPACKNAME)
	$(Q) touch $(UPF_UNPACKNAME)
	$(ECHO_END)

# Download and unpack tarball if no git repo found
ifeq ($(wildcard $(UPF_UNPACKNAME)/.git),)
context:: $(UPF_UNPACKNAME)
endif

include $(APPDIR)/Application.mk

ifeq ($(wildcard $(UPF_UNPACKNAME)/.git),)
distclean::
	$(call DELDIR, $(UPF_UNPACKNAME))
	$(call DELFILE, $(UPF_TARBALL))
endif
