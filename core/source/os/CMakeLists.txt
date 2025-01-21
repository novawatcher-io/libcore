add_library(
    os
    unix_cgroup.cc
    unix_countdown_latch.cc
    unix_current_thread.cc
    unix_pid_file.cc
    unix_sysinfo.cc
    unix_thread.cc
    unix_thread_proc.cc
    unix_timer.cc
    unix_timestamp.cc
    network_interface.cc)
add_library(libcore::os ALIAS os)

target_include_directories(os PUBLIC ${libcore_INCLUDE_DIR})
target_link_libraries(os PRIVATE libcore::event fmt::fmt spdlog::spdlog ${LIBCGROUP_LINK_LIBRARIES})
