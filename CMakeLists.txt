cmake_minimum_required(VERSION 3.5)

project(libalisp)
add_compile_options(-std=c11 -Wall -Werror -Wno-gnu-alignof-expression)

file(GLOB_RECURSE sources src/alisp/*.c src/alisp/types/*.c)

add_library(lib_static STATIC ${sources})
set_target_properties(lib_static PROPERTIES OUTPUT_NAME alisp)
target_include_directories(lib_static PUBLIC src/)

add_executable(alisp ${sources} src/main.c)
target_compile_options(alisp PUBLIC -O2 -g)
target_include_directories(alisp PUBLIC src/)

add_executable(tests ${sources} src/tests.c)
target_compile_options(tests PUBLIC -O2 -g)
target_include_directories(tests PUBLIC src/)
target_link_libraries(tests)

file(GLOB headers src/alisp/*.h)
install(FILES ${headers} DESTINATION include/alisp)

file(GLOB types_headers src/alisp/types/*.h)
install(FILES ${types_headers} DESTINATION include/alisp/types)

install(FILES build/libalisp.a DESTINATION lib)
install(FILES build/alisp DESTINATION bin PERMISSIONS WORLD_EXECUTE)
