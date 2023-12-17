set(target_name libclient)

add_library(
    ${target_name} STATIC
    connection.cpp 
    ${PROJECT_SOURCE_DIR}/include/client/connection.hpp
    key_gen.cpp 
    ${PROJECT_SOURCE_DIR}/include/client/key_gen.hpp
    authentication.cpp 
    ${PROJECT_SOURCE_DIR}/include/client/authentication.hpp
    ${PROJECT_SOURCE_DIR}/src/utils/utils.cpp
    ${PROJECT_SOURCE_DIR}/include/utils/utils.hpp
)

include(CompileOptions)
set_compile_options(${target_name})

target_link_libraries(
    ${target_name} 
    PRIVATE 
    cxxopts
)

target_include_directories(
    ${target_name} 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/external/
)
