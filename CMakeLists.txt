cmake_minimum_required(VERSION 3.23)

project(aos-launcher)

file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME} ${SRC_FILES})

find_package(CURL REQUIRED)
find_package(json-c REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} json-c)