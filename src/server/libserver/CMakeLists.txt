set(target_name libserver)

add_library(
    ${target_name} STATIC
    connection.cpp 
    ${PROJECT_SOURCE_DIR}/include/server/connection.hpp
)

include(CompileOptions)
set_compile_options(${target_name})

target_include_directories(
    ${target_name} 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/include/
)
