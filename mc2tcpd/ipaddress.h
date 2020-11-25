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
    };
    static constexpr IpV4 ANYv4{0U};

    struct IpV6 {
        uint32_t value4;
        uint32_t value3;
        uint32_t value2;
        uint32_t value1;
    };
    static constexpr IpV6 ANYv6{0U, 0U, 0U, 0U};

    enum class Family{ UNKNOWN, IPV4, IPV6};

    IpAddress():_v4(),_v6(){}

    explicit IpAddress(const IpV4& ipv4) noexcept
    : _v4(ipv4)
    , _v6()
    {};

    explicit IpAddress(const IpV6& ipv6 ) noexcept
    : _v4()
    , _v6(ipv6)
    {};

/*    explicit IpAddress(IpV4&& ipv4) noexcept
    : _v4(std::move(ipv4))
    , _v6()
    {};

    explicit IpAddress(IpV6&& ipv6) noexcept
            : _v4()
            , _v6(std::move(ipv6))
    {};

    IpAddress(IpAddress&& other) noexcept
    : _v4(std::move(other._v4))
    , _v6(std::move(other._v6))
    {};
*/
    Family family() const {return _v4.has_value() ? Family::IPV4 : ( _v6.has_value() ? Family::IPV6 : Family::UNKNOWN);}

    const IpV4& ipv4() const {return _v4.value();}
    const IpV6& ipv6() const {return _v6.value();}

    bool isAny() const {
        switch (family()){
            case Family::IPV4: return _v4.value().value == 0;
            case Family::IPV6: return (_v6.value().value1 | _v6.value().value2 | _v6.value().value3 | _v6.value().value4) == 0;
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
    IpEndpoint() noexcept
    : _port()
    , _ipAddress()
    {}

    IpEndpoint(const IpAddress& ip, const std::optional<uint16_t>& port) noexcept
    : _ipAddress(ip)
    , _port(port)
    {}

/*    IpEndpoint(IpAddress&& ip, std::optional<uint16_t>&& port) noexcept
    : _ipAddress(std::move(ip))
    , _port(std::move(port))
    {}*/

    [[nodiscard]] const IpAddress& ipAddress() const {return _ipAddress;}
    [[nodiscard]] const std::optional<uint16_t>& port() const {return _port;}

private:
    IpAddress _ipAddress;
    std::optional<uint16_t> _port;
};

bool operator==(const IpAddress::IpV4& lhs, const IpAddress::IpV4& rhs);
bool operator!=(const IpAddress::IpV4& lhs, const IpAddress::IpV4& rhs);
bool operator==(const IpAddress::IpV6& lhs, const IpAddress::IpV6& rhs);
bool operator!=(const IpAddress::IpV6& lhs, const IpAddress::IpV6& rhs);
bool operator==(const IpAddress& lhs, const IpAddress& rhs);
bool operator!=(const IpAddress& lhs, const IpAddress& rhs);
bool operator==(const IpEndpoint& lhs, const IpEndpoint& rhs);
bool operator!=(const IpEndpoint& lhs, const IpEndpoint& rhs);
std::istream& operator>>(std::istream& is, IpAddress::IpV4& ipv4);
std::ostream& operator<<(std::ostream& os, const IpAddress::IpV4& ipv4);
std::istream& operator>>(std::istream& is, IpAddress::IpV6& ipv6);
std::ostream& operator<<(std::ostream& os, const IpAddress::IpV6& ipv6);
std::istream& operator>>(std::istream& is, IpAddress& ipAddress);
std::ostream& operator<<(std::ostream& os, const IpAddress& ipAddress);
std::istream& operator>>(std::istream& is, IpEndpoint& ep);
std::ostream& operator<<(std::ostream& os, const IpEndpoint& ipEndpoint);

}
#endif //MC2TCPD_IPADDRESS_H
