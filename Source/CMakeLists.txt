cmake_minimum_required(VERSION 3.0)
project(PathTracer VERSION 1.0.0)

add_executable(PathTracer "")

set_target_properties(
    PathTracer
    PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_link_libraries(
    PathTracer
    PRIVATE
    $<$<PLATFORM_ID:Linux>:pthread>
    $<$<C_COMPILER_ID:GNU>:m>
)

target_sources(
    PathTracer
    PRIVATE
    bmp.c
    filesystem.c
    main.c
    memory.c
    random.c
    thread_pool.c
    vector_math.c
    $<$<PLATFORM_ID:Linux>:thread_pool_posix.c>
    $<$<C_COMPILER_ID:GNU>:atomic_gcc.c>
)