add_library(common helper.cc)
add_library(libcore::common ALIAS common)

target_include_directories(common PUBLIC ${libcore_INCLUDE_DIR})
target_link_libraries(common PRIVATE spdlog::spdlog ${ZLIB_LIBRARIES})
