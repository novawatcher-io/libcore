#include "os/network_interface.h"
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unordered_map>
#include <string>

namespace Core::OS {
bool GetHostNetworkCard(std::unordered_map<std::string, IpInfo>& ip_map) {
    struct ifaddrs* interfaces = nullptr;

    // Retrieve the current interfaces
    if (getifaddrs(&interfaces) == -1) {
        SPDLOG_ERROR("Error getting interfaces");
        return false;
    }

    auto* temp_addr = interfaces;
    while (temp_addr != nullptr) {
        // Exclude loopback and Docker interfaces
        if (strcmp(temp_addr->ifa_name, "lo") == 0 || strncmp(temp_addr->ifa_name, "docker", 6) == 0 ||
            strncmp(temp_addr->ifa_name, "br-", 3) == 0) {
            temp_addr = temp_addr->ifa_next;
            continue;
        }

        // Check if the interface is up
        if (!(temp_addr->ifa_flags & IFF_UP)) {
            temp_addr = temp_addr->ifa_next;
            continue;
        }

        char address[INET6_ADDRSTRLEN];
        memset(address, 0, sizeof(address));
        const char* ip;

        if (temp_addr->ifa_addr->sa_family == AF_INET6) {
            void* addr_ptr = &((struct sockaddr_in6*)temp_addr->ifa_addr)->sin6_addr;
            ip = inet_ntop(temp_addr->ifa_addr->sa_family, addr_ptr, address, sizeof(address));
            ip_map[temp_addr->ifa_name].ipv6 = ip;
        } else if (temp_addr->ifa_addr->sa_family == AF_INET) {
            void* addr_ptr = &((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr;
            ip = inet_ntop(temp_addr->ifa_addr->sa_family, addr_ptr, address, sizeof(address));
            ip_map[temp_addr->ifa_name].ipv4 = ip;
        }

        temp_addr = temp_addr->ifa_next;
    }
    // Free the memory allocated by getifaddrs
    freeifaddrs(interfaces);
    return true;
}

bool GetOneIpInfo(IpInfo& result) {
    std::unordered_map<std::string, IpInfo> ip_map;
    if (!GetHostNetworkCard(ip_map)) {
        return false;
    }

    for (auto& [name, ip] : ip_map) {
        SPDLOG_INFO("interface: {}, ipv6: {}, ipv4: {}", name, ip.ipv4, ip.ipv6);
    }

    for (auto& [name, ip] : ip_map) {
        if (!ip.ipv4.empty() && !ip.ipv6.empty()) {
            SPDLOG_INFO("use interface: {}, ipv4: {}, ipv6: {}", name, ip.ipv4, ip.ipv6);
            result = ip;
        } else if (!ip.ipv6.empty()) {
            result.ipv6 = ip.ipv6; // random pick one
        } else if (!ip.ipv4.empty()) {
            result.ipv4 = ip.ipv4; // random pick one
        }
    }
    return true;
}
} // namespace Core::OS