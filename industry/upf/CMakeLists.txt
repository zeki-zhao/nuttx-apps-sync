# ##############################################################################
# apps/industry/upf/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_INDUSTRY_UPF)

  # ############################################################################
  # Config and Fetch UPF
  # ############################################################################
  set(UPF_DIR ${CMAKE_CURRENT_LIST_DIR}/upf)

  set(UPF_VERSION
      "${CONFIG_UPF_VERSION_MAJOR}.${CONFIG_UPF_VERSION_MINOR}.${CONFIG_UPF_VERSION_PATCH}"
  )

  if(NOT EXISTS ${UPF_DIR})
    set(UPF_URL
        "https://gitee.com/zeki_zhao/universal-protocol-framework/repository/archive/release-v${UPF_VERSION}.zip")
    FetchContent_Declare(
      upf_fetch
      URL ${UPF_URL}
          DOWNLOAD_DIR
          ${CMAKE_CURRENT_LIST_DIR}
          SOURCE_DIR
          ${CMAKE_CURRENT_LIST_DIR}/upf
          BINARY_DIR
          ${CMAKE_BINARY_DIR}/apps/industry/upf/upf
      DOWNLOAD_NO_PROGRESS true
      TIMEOUT 120)

    FetchContent_GetProperties(upf_fetch)

    if(NOT upf_fetch_POPULATED)
      FetchContent_Populate(upf_fetch)
    endif()
  endif()

  # ############################################################################
  # Library Configuration
  # ############################################################################

  file(GLOB_RECURSE SRCS "${UPF_DIR}/*.S" "${UPF_DIR}/*.c" "${UPF_DIR}/*.cpp")
  if(NOT CONFIG_UPF_DEMO)
    list(FILTER SRCS EXCLUDE REGEX "/Demo/|/DemoInstance/")
  endif()

  nuttx_add_library(upf)
  target_sources(upf PRIVATE ${SRCS})

  set(UPF_SUBDIRS
    ${UPF_DIR}/Config
    ${UPF_DIR}/ParameterManager
    ${UPF_DIR}/ProtocolComponents
    ${UPF_DIR}/ProtocolManager)

  if(CONFIG_UPF_DEMO)
    list(APPEND UPF_SUBDIRS ${UPF_DIR}/DemoInstance ${UPF_DIR}/Demo)
  endif()

  target_include_directories(upf PRIVATE ${UPF_DIR} ${UPF_SUBDIRS})

  set(NUTTX_UPF_INCDIRS
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/upf
    ${CMAKE_CURRENT_LIST_DIR}/upf/Config
    ${CMAKE_CURRENT_LIST_DIR}/upf/ParameterManager
    ${CMAKE_CURRENT_LIST_DIR}/upf/ProtocolComponents
    ${CMAKE_CURRENT_LIST_DIR}/upf/ProtocolManager)

  if(CONFIG_UPF_DEMO)
    list(APPEND NUTTX_UPF_INCDIRS
      ${CMAKE_CURRENT_LIST_DIR}/upf/DemoInstance
      ${CMAKE_CURRENT_LIST_DIR}/upf/Demo)
  endif()

  set_property(TARGET nuttx APPEND PROPERTY NUTTX_INCLUDE_DIRECTORIES
    ${NUTTX_UPF_INCDIRS})

endif()
