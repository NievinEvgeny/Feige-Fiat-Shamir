set(target_name libclient)

add_library(
    ${target_name} STATIC
    connection.cpp 
    ${PROJECT_SOURCE_DIR}/include/client/connection.hpp
    ${PROJECT_SOURCE_DIR}/src/utils/utils.cpp
    ${PROJECT_SOURCE_DIR}/include/utils/utils.hpp
)

include(CompileOptions)
set_compile_options(${target_name})

target_include_directories(
    ${target_name} 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/include/
)
