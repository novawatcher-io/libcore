#pragma once

#warning "Do not use this header file directly."

extern "C" {
#include <jemalloc/jemalloc.h>
}

/**
 * @brief 对外的api收口头文件，引入这个头文件后，就可以使用全部的类库了
 *
 */
#include "build_expect.h"
#include "non_copyable.h"
#include "non_moveable.h"

#include "os/unix_sysinfo.h"
#include "os/unix_sigset.h"
#include "os/unix_current_thread.h"
#include "os/unix_pid_file.h"
#include "event/event_buffer_channel.h"
#include "event/event_no_buffer_channel.h"
#include "os/unix_process_daemonize.h"
#include "component/udp_server_channel.h"
#include "component/timer_channel.h"
#include "component/thread_container.h"
#include "component/process/manager.h"
