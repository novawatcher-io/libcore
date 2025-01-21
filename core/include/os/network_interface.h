#pragma once
#include <string>
#include <unordered_map>

namespace Core::OS {
struct IpInfo {
    std::string ipv4;
    std::string ipv6;
};

bool GetHostNetworkCard(std::unordered_map<std::string, IpInfo>& ip_map);
bool GetOneIpInfo(IpInfo& result);
} // namespace Core::OS