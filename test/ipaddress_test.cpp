#include "gtest/gtest.h"
#include "ipaddress.h"

using namespace Networking;

TEST(IpAddress, v4value)
{
    const uint32_t ipValue=1;
    IpAddress::IpV4 ip{ipValue};
    EXPECT_EQ(ipValue, ip.value);

    const uint32_t ipValue2=2;
    IpAddress::IpV4 ip2{ipValue2};
    EXPECT_EQ(ipValue2, ip2.value);

    IpAddress::IpV4 ipAny = IpAddress::ANYv4;
    EXPECT_EQ(IpAddress::ANYv4.value, ipAny.value);

    EXPECT_EQ(IpAddress::IpV4{ipValue}, ip);
    EXPECT_EQ(IpAddress::IpV4{ipValue2}, ip2);
    EXPECT_EQ(IpAddress::ANYv4, ipAny);

    EXPECT_NE(ip, ip2);
    EXPECT_NE(ipAny, ip);
    EXPECT_NE(ipAny, ip2);
}


TEST(IpAddress, ctor)
{
    {
        IpAddress ipAddress;
    }

    {
        const IpAddress::IpV4 ipv4{1};
        IpAddress ipAddress(ipv4);
    }

    {
        const IpAddress::IpV6 ipv6{1,1,1,1};
        IpAddress ipAddress(ipv6);
    }
}

TEST(IpAddress, family)
{
    {
        IpAddress ipAddress;
        EXPECT_EQ(IpAddress::Family::UNKNOWN, ipAddress.family());
    }

    {
        IpAddress ipAddress(IpAddress::IpV4{0});
        EXPECT_EQ(IpAddress::Family::IPV4, ipAddress.family());
    }

    {
        IpAddress ipAddress(IpAddress::IpV6{0,0,0,0});
        EXPECT_EQ(IpAddress::Family::IPV6, ipAddress.family());
    }
}

TEST(IpAddress, get_ipv4)
{
    {
        IpAddress ipAddress;
        try {
            ipAddress.ipv4();
            FAIL();
        }
        catch (const std::exception&)
        {}
    }
    {
        const IpAddress::IpV6 ipv6{1,1,1,1};
        IpAddress ipAddress(ipv6);
        try {
            ipAddress.ipv4();
            FAIL();
        }
        catch (const std::exception&)
        {}
    }
    {
        const IpAddress::IpV4 ipv4{1};
        IpAddress ipAddress(ipv4);
        EXPECT_EQ(ipv4, ipAddress.ipv4());
    }
}

TEST(IpAddress, get_ipv6)
{
    {
        IpAddress ipAddress;
        try {
            ipAddress.ipv6();
            FAIL();
        }
        catch (const std::exception&)
        {}
    }
    {
        const IpAddress::IpV4 ipv4{1};
        IpAddress ipAddress(ipv4);
        try {
            ipAddress.ipv6();
            FAIL();
        }
        catch (const std::exception&)
        {}
    }
    {
        const IpAddress::IpV6 ipv6{1,1,1,1};
        IpAddress ipAddress(ipv6);
        EXPECT_EQ(ipv6, ipAddress.ipv6());
    }
}

TEST(IpAddress, isAny)
{
    {
        IpAddress ipAddress;
        EXPECT_FALSE(ipAddress.isAny());
    }

    {
        const IpAddress::IpV4 ipv4{1};
        IpAddress ipAddress(ipv4);
        EXPECT_FALSE(ipAddress.isAny());
    }

    {
        const IpAddress::IpV6 ipv6{1,1,1,1};
        IpAddress ipAddress(ipv6);
        EXPECT_FALSE(ipAddress.isAny());
    }
    {
        const IpAddress::IpV4 ipv4 = IpAddress::ANYv4;
        IpAddress ipAddress(ipv4);
        EXPECT_TRUE(ipAddress.isAny());
    }

    {
        const IpAddress::IpV6 ipv6 = IpAddress::ANYv6;
        IpAddress ipAddress(ipv6);
        EXPECT_TRUE(ipAddress.isAny());
    }
}

template <class ET>
struct TestParam
{
    IpAddress ip1;
    IpAddress ip2;
    ET expected;

};

class IpAddressParamTest : public testing::TestWithParam<TestParam<bool>>
{

};

TEST_P(IpAddressParamTest, op_eqTest)
{
    const TestParam<bool>& param = GetParam();
    EXPECT_EQ(param.expected, param.ip1 == param.ip2);
    EXPECT_NE(param.expected, param.ip1 != param.ip2);
}

INSTANTIATE_TEST_CASE_P(op_eq,
        IpAddressParamTest,
        testing::Values(
                TestParam<bool>{IpAddress(IpAddress::IpV4{0}), IpAddress(IpAddress::IpV4{0}), true},
                TestParam<bool>{IpAddress(IpAddress::IpV4{0}), IpAddress(IpAddress::IpV4{1}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV4{1}), IpAddress(IpAddress::IpV4{1}), true},
                TestParam<bool>{IpAddress(IpAddress::IpV6{0,0,0,0}), IpAddress(IpAddress::IpV4{0}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV4{0}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{0,0,0,0}), IpAddress(IpAddress::IpV4{1}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV4{1}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{0,0,0,0}), IpAddress(IpAddress::IpV6{0,0,0,0}), true},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV6{1,1,1,1}), true},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV6{1,1,1,0}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV6{1,1,0,1}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV6{1,0,1,1}), false},
                TestParam<bool>{IpAddress(IpAddress::IpV6{1,1,1,1}), IpAddress(IpAddress::IpV6{0,1,1,1}), false}
                )
        );