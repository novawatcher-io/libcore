add_subdirectory(process)

add_library(component thread_container.cc)
add_library(libcore::component ALIAS component)

target_include_directories(component PUBLIC ${libcore_INCLUDE_DIR})
target_link_libraries(component PRIVATE spdlog::spdlog fmt::fmt libcore::event)
