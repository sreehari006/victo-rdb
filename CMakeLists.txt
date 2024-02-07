cmake_minimum_required(VERSION 3.27.1)

project(victo VERSION 1.0.1)

# Set the build type to Debug
set(CMAKE_BUILD_TYPE Debug)

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL: Version ${OPENSSL_VERSION}")
message(STATUS "OpenSSL: include dir at ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL: libraries at ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES}")

include_directories(${OPENSSL_INCLUDE_DIR})

# add_library(${PROJECT_NAME} SHARED main.c)
add_executable(${PROJECT_NAME}-exe main.c)

# Add the -g flag for debugging information
target_compile_options(${PROJECT_NAME}-exe PRIVATE -g)

add_subdirectory(src)


# target_link_libraries(${PROJECT_NAME} src core servers utils m ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})
target_link_libraries(${PROJECT_NAME}-exe src m ${OPENSSL_LIBRARIES} ${OPENSSL_SSL_LIBRARIES})





