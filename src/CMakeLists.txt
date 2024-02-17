aux_source_directory(. SRC_LIST)

add_executable(demo ${SRC_LIST} ../main.cpp)

target_link_libraries(demo pthread)

