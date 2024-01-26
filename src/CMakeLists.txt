file(GLOB_RECURSE SOURCES "*.c")
file(GLOB HEADERS "*.h")

add_library(src ${SOURCES})