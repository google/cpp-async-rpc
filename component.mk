# *** component.mk file for the use of lasr as an ESP-IDF component.
#
# Copyright 2019 by Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain a
# copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

COMPONENT_SRCDIRS := src src/lasr src/lasr/traits src/lasr/testing src/lasr/container src/lasr/container/detail
COMPONENT_ADD_INCLUDEDIRS := src third_party/function2/include
CXXFLAGS += -std=c++17
