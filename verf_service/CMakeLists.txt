project(verf_service VERSION 0.5 LANGUAGES CXX)

################################################################################
# Get current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get latest abbreviated commit hash of working branch
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get count of commits in working branch
execute_process(
        COMMAND git rev-list --count HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Git current branch: ${GIT_BRANCH}")
message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
message(STATUS "Git commit count: ${GIT_COUNT}")

message(STATUS "Generating version.h")

add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")
add_definitions("-DGIT_COUNT=${GIT_COUNT}")

configure_file(
        ${PROJECT_SOURCE_DIR}/src/version.h.in
        ${PROJECT_BINARY_DIR}/generated/version.h
)

include_directories(${PROJECT_BINARY_DIR}/generated)

################################################################################

add_executable(${PROJECT_NAME}
        src/main.cpp
        )

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} meta_log)
target_link_libraries(${PROJECT_NAME} meta_transaction)
target_link_libraries(${PROJECT_NAME} meta_crypto)
target_link_libraries(${PROJECT_NAME} meta_connections)
target_link_libraries(${PROJECT_NAME} meta_pool)
target_link_libraries(${PROJECT_NAME} meta_network)
target_link_libraries(${PROJECT_NAME} meta_connections)
target_link_libraries(${PROJECT_NAME} http_server)
target_link_libraries(${PROJECT_NAME} moodycamel)
target_link_libraries(${PROJECT_NAME} rapidjson)
