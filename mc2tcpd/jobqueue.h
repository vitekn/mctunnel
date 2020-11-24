#ifndef MC2TCPD_JOBQUEUE_H
#define MC2TCPD_JOBQUEUE_H
#include <deque>
#include <mutex>
#include <functional>

class JobQueue
{
public:
    void postJob(std::function<void()>&& fn)
    {
        const std::lock_guard<std::mutex> lock(_mutex);
        _jobs.emplace_back(std::move(fn));
    }

    void postJob(const std::function<void()>& fn)
    {
        const std::lock_guard<std::mutex> lock(_mutex);
        _jobs.emplace_back(fn);
    }

    bool dispatchJob()
    {
        std::function<void()> fn;
        {
            const std::lock_guard <std::mutex> lock(_mutex); //queue
            if (!_jobs.empty()) {
                fn = _jobs.front();
                _jobs.pop_front();
            }
        }
        if (fn) {
            fn();
            return true;
        }
        return false;
    }

private:
    std::deque<std::function<void()>> _jobs;
    std::mutex _mutex;
};

#endif //MC2TCPD_JOBQUEUE_H
