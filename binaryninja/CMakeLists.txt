# Copyright 2019-2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(${binexport_bn_plugin_name} SHARED
  main_plugin.cc
  main_plugin.h
  log_sink.cc
  log_sink.h
)
target_link_libraries(${binexport_bn_plugin_name} PRIVATE
  absl::cleanup
  absl::log
  absl::log_initialize
  absl::status
  absl::statusor
  absl::strings
  BinaryNinja::API
  binexport_core
)
set_target_properties(${binexport_bn_plugin_name} PROPERTIES
  PREFIX ""
)

if(BN_INTERNAL_BUILD)
  # When building as part of Binary Ninja, output to the correct plugin directory
  set_target_properties(${binexport_bn_plugin_name} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BN_CORE_PLUGIN_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${BN_CORE_PLUGIN_DIR}
  )
  if(APPLE)
    ui_plugin_rpath(${binexport_bn_plugin_name})
  elseif(UNIX)
    ui_plugin_rpath(${binexport_bn_plugin_name})
  endif()
else()
  install(TARGETS ${binexport_bn_plugin_name}
          ARCHIVE DESTINATION binexport-prefix
          RUNTIME DESTINATION binexport-prefix
          LIBRARY DESTINATION binexport-prefix)
endif()
