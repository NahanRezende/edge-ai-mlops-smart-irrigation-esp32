cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ia_automated_irrigation_system)

if(NOT DEFINED PROJECT_VER)
    # Valor de segurança local (ex.: build manual fora do GitHub)
    set(PROJECT_VER "local-dev")
endif()
