file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")

add_library(Mem_DB CMemory_Database.cpp ${HEADER_LIST})

target_include_directories(Mem_DB PUBLIC ../include/)

target_compile_features(Mem_DB PUBLIC cxx_std_20)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
