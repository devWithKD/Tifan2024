# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/kedar/pico/pico-sdk/tools/pioasm"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pioasm"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kedar/Personal_Project/PWM_PLAYGROUND/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
