set(B_TARGET kiv-cpp-sp-02)
add_executable(${B_TARGET} main.cpp)
target_link_libraries(${B_TARGET} PRIVATE Mem_DB)

