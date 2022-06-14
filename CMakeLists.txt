cmake_minimum_required(VERSION 3.14)

project(pinked C)

include(FetchContent)

FetchContent_Declare(
	cglm
	GIT_REPOSITORY	https://github.com/recp/cglm
	GIT_TAG			v0.8.4
)

set(CGLM_STATIC ON CACHE INTERNAL "")
set(CGLM_USE_C99 ON CACHE INTERNAL "")
FetchContent_MakeAvailable(cglm)

FetchContent_Declare(
	hashtable
	GIT_REPOSITORY	https://github.com/xtreme8000/hashtable
	GIT_TAG			master
)

FetchContent_MakeAvailable(hashtable)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS EGL)

add_executable(pinked
				src/pinked.c
				src/chunk.c
				src/input_stream.c
				src/layer.c
				src/output_stream.c
			)

set_target_properties(
	pinked PROPERTIES
	C_STANDARD 99
)

target_link_libraries(pinked cglm SDL2::SDL2 OpenGL::GL OpenGL::EGL hashtable-static m)
