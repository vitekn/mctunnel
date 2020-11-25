#include "ipaddress.h"
namespace Networking
{

bool operator==(const IpAddress::IpV4& lhs, const IpAddress::IpV4& rhs)
{
    return lhs.value == rhs.value;
}

bool operator!=(const IpAddress::IpV4& lhs, const IpAddress::IpV4& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const IpAddress::IpV6& lhs, const IpAddress::IpV6& rhs)
{
    return lhs.value4 == rhs.value4 && lhs.value3 == rhs.value3 && lhs.value2 == rhs.value2 && lhs.value1 == rhs.value1;
}

bool operator!=(const IpAddress::IpV6& lhs, const IpAddress::IpV6& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const IpAddress& lhs, const IpAddress& rhs)
{
    if (lhs.family() == IpAddress::Family::UNKNOWN || lhs.family() != rhs.family()) {
        return false;
    }
    if (lhs.family() == IpAddress::Family::IPV4) {
        return lhs.ipv4() == rhs.ipv4();
    } else {
        return lhs.ipv6() == rhs.ipv6();
    }
}

bool operator!=(const IpAddress& lhs, const IpAddress& rhs)
{
    return !(lhs == rhs);
}

bool operator==(const IpEndpoint& lhs, const IpEndpoint& rhs)
{
    return lhs.ipAddress() == rhs.ipAddress() && lhs.port() == rhs.port();
}

bool operator!=(const IpEndpoint& lhs, const IpEndpoint& rhs)
{
    return !(lhs == rhs);
}

std::istream& operator>>(std::istream& is, IpAddress::IpV4& ipv4)
{
    char dots[3];
    unsigned int oct[4];
    is >> oct[0] >> dots[0] >> oct[1] >> dots[1] >> oct[2] >> dots[2] >> oct[3];
    bool inRange = ((oct[0] | oct[1] | oct[2] | oct[3]) & 0xFFFFFF00) == 0;
    bool allDots = dots[0] == '.' && dots[1] == '.' && dots[2] == '.';
    if (is && allDots && inRange) {
        ipv4 = IpAddress::IpV4 { (oct[0]<<24) | (oct[1] << 16) | (oct[2] << 8) | oct[3] };
        return is;
    }
    is.setstate(std::ios_base::failbit);
    return is;
}

std::ostream& operator<<(std::ostream& os, const IpAddress::IpV4& ipv4)
{
    os <<  (ipv4.value >> 24)  << '.'
       << ((ipv4.value >> 16) & 0xFF) << '.'
       << ((ipv4.value >> 8)  & 0xFF) << '.'
       <<  (ipv4.value        & 0xFF);
    return os;
}

std::istream& operator>>(std::istream& is, IpAddress::IpV6& ipv6)
{
    uint16_t parts[8];
    is >> std::hex >> parts[0];
    for (int i=1; i<8 && is; ++i){
        char col;
        is >> col;
        if (col !=':') {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        is >> parts[i];
    }
    if (is) {
        ipv6 = IpAddress::IpV6{(((uint32_t)(parts[0]))<<16) | (uint32_t)parts[1],
                               ((uint32_t)(parts[2])<<16) | (uint32_t)parts[3],
                               ((uint32_t)(parts[4])<<16) | (uint32_t)parts[5],
                               ((uint32_t)(parts[6])<<16) | (uint32_t)parts[7]};
        return is;
    }
    is.setstate(std::ios_base::failbit);
    return is;
}

std::ostream& operator<<(std::ostream& os, const IpAddress::IpV6& ipv6)
{
    os << std::hex
       << (ipv6.value4 >> 16)  << ':' << (ipv6.value4 & 0xFFFF) << ':'
       << (ipv6.value3 >> 16)  << ':' << (ipv6.value3 & 0xFFFF) << ':'
       << (ipv6.value2 >> 16)  << ':' << (ipv6.value2 & 0xFFFF) << ':'
       << (ipv6.value1 >> 16)  << ':' << (ipv6.value1 & 0xFFFF) << ':';
    return os;
}

std::istream& operator>>(std::istream& is, IpAddress& ipAddress)
{
    std::string tmp;
    is >> tmp;
    std::string::size_type dotPos = tmp.find_first_of('.');
    if (dotPos != std::string::npos) {
        if (dotPos > 0 &&  dotPos <= 3) {
            std::istringstream  iss(tmp);
            IpAddress::IpV4 ipv4;
            iss >> ipv4;
            if (iss) {
                ipAddress = IpAddress(ipv4);
                return is;
            }
        }
    } else {
        std::string::size_type colPos = tmp.find_first_of(':');
        if (colPos != std::string::npos) {
            std::istringstream  iss(tmp);
            IpAddress::IpV6 ipv6;
            iss >> ipv6;
            if (iss) {
                ipAddress = IpAddress(ipv6);
                return is;
            }
        }
    }

    is.setstate(std::ios_base::failbit);
    return is;
}

std::ostream& operator<<(std::ostream& os, const IpAddress& ipAddress)
{
    switch (ipAddress.family()) {
        case  IpAddress::Family::UNKNOWN:
            os << "UNKNOWN_IP_ADDRESS";
            break;
        case  IpAddress::Family::IPV4:
            os << ipAddress.ipv4();
            break;
        case  IpAddress::Family::IPV6:
            os << ipAddress.ipv6();
            break;
    }
    return os;
}

std::istream& operator>>(std::istream& is, IpEndpoint& ep)
{
    std::string tmp;
    is >> tmp;
    std::string::size_type lastColPos = tmp.find_last_of(':');
    if (lastColPos != std::string::npos) {
        std::istringstream iss(tmp);
        IpAddress ipAddress;
        iss >> ipAddress;
        if (iss.good()) {
            char col;
            uint32_t port;
            iss >> col;
            if (iss && col == ':') {
                iss >> port;
                if (iss && port < 0x10000) {
                    ep = IpEndpoint(ipAddress, port);
                    return is;
                }
            }
        }
    }

    is.setstate(std::ios_base::failbit);
    return is;
}

std::ostream& operator<<(std::ostream& os, const IpEndpoint& ipEndpoint)
{
    os << ipEndpoint.ipAddress() << ':' << ipEndpoint.port().value_or(0);
    return os;
}


}
