#ifndef MC2TCPD_ONSCOPEEXIT_H
#define MC2TCPD_ONSCOPEEXIT_H
#include <functional>

class OnScopeExit
{
public:
    OnScopeExit(std::function<void(*)()>&& functor)
    : f(std::move(functor))
    {}

    OnScopeExit(const std::function<void(*)()>& functor)
    : f(functor)
    {}

    ~OnScopeExit()
    {
        f();
    }

    OnScopeExit(const OnScopeExit&) = delete;
    OnScopeExit(OnScopeExit&&) = delete;
    OnScopeExit& operator=(const OnScopeExit&) = delete;

private:
    std::function<void(*)()> f;
};


#endif //MC2TCPD_ONSCOPEEXIT_H
