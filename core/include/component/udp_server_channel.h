#pragma once

#include "os/unix_udp_server.h"
#include "event/event_no_buffer_channel.h"

namespace Core {
namespace Component {

/**
 * @brief udp 服务器的 通讯管道
 * 
 */
class UdpServerChannel :public OS::UnixUdpServer, public  Event::EventNoBufferChannel {

public:
    UdpServerChannel(const std::shared_ptr<Event::EventLoop> &loop_, const std::string& ip, uint16_t port) 
    :OS::UnixUdpServer(ip, port)
    ,Event::EventNoBufferChannel(loop_, getFd()) {
        
    }

    ~UdpServerChannel() {

    }

private:

};
}
}
