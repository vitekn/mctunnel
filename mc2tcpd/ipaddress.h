#ifndef MC2TCPD_IPADDRESS_H
#define MC2TCPD_IPADDRESS_H
#include <optional>
#include <cstdint>
#include <istream>
#include <ostream>
#include <sstream>

namespace Networking
{
class IpAddress
{
public:
    struct IpV4 {
        uint32_t value;
        static constexpr IpV4 ANY{0U};
    };

    struct IpV6 {
        uint32_t value4;
        uint32_t value3;
        uint32_t value2;
        uint32_t value1;
        static constexpr IpV6 ANY{0U, 0U, 0U, 0U};
    };
    enum class Family{ UNKNOWN, IPV4, IPV6};

    IpAddress():_v4(),_v6(){}

    IpAddress(const IpV4& ipv4) noexcept
    : _v4(ipv4)
    , _v6()
    {};
    IpAddress(const IpV6& ipv6 ) noexcept
    : _v4()
    , _v6(ipv6)
    {};
    IpAddress(IpV4&& ipv4) noexcept
    : _v4(std::move(ipv4))
    , _v6()
    {};
    IpAddress(IpV6&& ipv6) noexcept
            : _v4()
            , _v6(std::move(ipv6))
    {};
    IpAddress(IpAddress other) noexcept
    : _v4(std::move(other._v4))
    , _v6(std::move(other._v6))
    {};

    Family family() const {return _v4 ? Family::IPV4 : ( _v6 ? Family::IPV6 : Family::UNKNOWN);}

    const IpV4& ipv4() const {return _v4.value();}
    const IpV6& ipv6() const {return _v6.value();}

    bool isAny const {
        switch (family()){
            case Family::IPV4: return _v4.value == IpV4::ANY;
            case Family::IPV6: return _v6.value == IpV6::ANY;
            default: break;
        }
        return false;
    };

private:
    std::optional<IpV4> _v4;
    std::optional<IpV6> _v6;
};

class IpEndpoint
{
public:
    IpEndpoint(IpEndpoint&& other) noexcept
    : _port(other.port())
    , _ipAddress(std::move(other._ipAddress))
    {}

    IpEndpoint(const IpAddress& ip, const std::optional<uint16_t>& port) noexcept
    : _ipAddress(ip)
    , _port(port)
    {}
    IpEndpoint(IpAddress&& ip, std::optional<uint16_t>&& port) noexcept
    : _ipAddress(std::move(ip))
    , _port(std::move(port))
    {}

    const IpAddress& ipAddress() const {return _ipAddress;}
    const std::optional<uint16_t>& port() const {return _port;}

private:
    IpAddress _ipAddress;
    std::optional<uint16_t> _port;
};

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

bool operator==(const IpEndpoint& lhs, const IpEndpoint& rhs)
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
    if (is.good() && allDots && inRange) {
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
std::ostream& operator<<(std::ostream& os, const IpAddress::IpV6& ipv6)
{
    os << std::hex << reinterpret_cast<uint16_t>(ipv6.value4 >> 16)  << ':' << reinterpret_cast<uint16_t>(ipv6.value4 & 0xFFFF) << ':'
                   << reinterpret_cast<uint16_t>(ipv6.value3 >> 16)  << ':' << reinterpret_cast<uint16_t>(ipv6.value3 & 0xFFFF) << ':'
                   << reinterpret_cast<uint16_t>(ipv6.value2 >> 16)  << ':' << reinterpret_cast<uint16_t>(ipv6.value2 & 0xFFFF) << ':'
                   << reinterpret_cast<uint16_t>(ipv6.value1 >> 16)  << ':' << reinterpret_cast<uint16_t>(ipv6.value1 & 0xFFFF) << ':';
    return os;
}

std::istream& operator>>(std::istream& is, IpAddress::IpV6& ipv6)
{
    uint16_t parts[8];
    is >> std::hex >> parts[0];
    for (int i=1; i<8 && iss.good(); ++i){
        char col;
        is >> col
        if (col !=':') {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        is >> parts[i];
    }
    if (is.good()) {
        ipv6 = IpAddress::IpV6{(parts[0]<<8)|parts[1],
                               (parts[2]<<8)|parts[3],
                               (parts[4]<<8)|parts[5],
                               (parts[6]<<8)|parts[7]};
        return is;
    }
    is.setstate(std::ios_base::failbit);
    return is;
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
            if (iss.good()) {
                ipAddress = IpAddress(std::move(ipv4));
                return is;
            }
        }
    } else {
        std::string::size_type colPos = tmp.find_first_of(':');
        if (colPos != std::string::npos) {
            std::istringstream  iss(tmp);
            IpAddress::IpV6 ipv6;
            iss >> ipv6;
            if (iss.good()) {
                ipAddress = IpAddress(std::move(ipv6));
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

std::istream& operator>>(istream& is, IpEndpoint& ep)
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
            if (iss.good() && col == ':') {
                iss >> port;
                if ( iss.good() && port < 0x10000) {
                    ep = IpEndpoint(std::move(ipAddress), port);
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
    os << ipEndpoint.ipAddress() << ':' << ipEndpoint.port().value_or("0");
    return os;
}

}
#endif //MC2TCPD_IPADDRESS_H