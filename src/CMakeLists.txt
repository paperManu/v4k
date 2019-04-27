#
# Copyright (C) Emmanuel Durand
#
# This file is part of v4k.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Splash is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Splash.  If not, see <http://www.gnu.org/licenses/>.
#

#
# Enable all warnings as errors
#
add_definitions(-Wall -Wextra -Werror)
add_definitions(-Wno-unknown-pragmas -Wno-missing-field-initializers)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

include_directories(
    ./
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
)

link_directories(
    ${GLFW_LIBRARY_DIRS}
    ${VULKAN_LIBRARY_DIRS}
)

#
# Targets
#
add_executable(v4k)

target_sources(v4k PRIVATE
  v4k.cpp
)

target_link_libraries(v4k
    ${GLFW_LIBRARIES}
    ${VULKAN_LIBRARIES}
)
