# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

set(COMPONENT_REQUIRES )

set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
"main.cpp"
"../lib/SSD1283A/src/SSD1283A.cpp"
)

set(COMPONENT_ADD_INCLUDEDIRS
"../lib/IR_Blaster"
"../lib/IR_Blaster/Example"
"../lib/CleanRTOS/src"
"../lib/SSD1283A/src"
"../lib/SSD1283A/examples"

)

register_component()

#idf_component_register(SRCS ${app_sources})

