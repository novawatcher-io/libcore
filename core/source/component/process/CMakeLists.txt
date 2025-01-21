add_library(process manager.cc process.cc)
add_library(libcore::process ALIAS process)

target_include_directories(process PUBLIC ${libcore_INCLUDE_DIR})
target_link_libraries(process PRIVATE spdlog::spdlog fmt::fmt)
