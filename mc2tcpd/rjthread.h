#ifndef MC2TCPD_RJTHREAD_H
#define MC2TCPD_RJTHREAD_H
#include <thread>
#include <functional>
#include <future>
#include <atomic>

class RjThread {
public:
    RjThread(std::function<void()> runFn)
    : _runFn(std::move(runFn))
    , _exec(true)
    , _thread([this](){this->run();})
    {};

    ~RjThread()
    {
        _exec.store(false);
        _thread.join();
    }

private:

    void run()
    {
        while (_exec) {
            _runFn();
        }
    }


    std::function<void()> _runFn;
    std::atomic_bool _exec;
    std::thread _thread;
}


#endif //MC2TCPD_RJTHREAD_H
