set(server_name server)

add_executable(${server_name})

include(CompileOptions)
set_compile_options(${server_name})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_sources(
    ${server_name} 
    PRIVATE 
    server.cpp
)

target_link_libraries(
    ${server_name} 
    PUBLIC 
    libserver 
    Threads::Threads
    cxxopts
)

target_include_directories(
    ${server_name} 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/include/
)

add_subdirectory(libserver)
