# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/nahan/esp/esp-idf/components/bootloader/subproject"
  "/home/nahan/Documents/tcc_repository/build/bootloader"
  "/home/nahan/Documents/tcc_repository/build/bootloader-prefix"
  "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/tmp"
  "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/src/bootloader-stamp"
  "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/src"
  "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/nahan/Documents/tcc_repository/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
