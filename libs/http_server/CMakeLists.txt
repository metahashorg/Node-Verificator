project(http_server LANGUAGES CXX)

add_library(${PROJECT_NAME}
        src/http_server.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)

target_link_libraries(${PROJECT_NAME} meta_log)
target_link_libraries(${PROJECT_NAME} boost_coroutine)