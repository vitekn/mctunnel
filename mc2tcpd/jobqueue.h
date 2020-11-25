#ifndef MC2TCPD_JOBQUEUE_H
#define MC2TCPD_JOBQUEUE_H
#include <deque>
#include <mutex>
#include <functional>

class JobQueue
{
struct JobFuncErasure
{
    virtual void operator()() = 0;
    virtual ~JobFuncErasure() = default;
};
template <class F>
struct JobFunc : JobFuncErasure
{
    JobFunc(F&& f): _f(std::forward<F>(f)) {}
    void operator()() override { _f();}
    F _f;
};

public:

    template<class F>
    void postJob(F&& f)
    {
        const std::lock_guard<std::mutex> lock(_mutex);
        std::unique_ptr<JobFuncErasure> fn = std::make_unique<JobFunc<F>>(std::forward<F>(f));
        _jobs.emplace_back(std::move(fn));
    }

    bool dispatchJob()
    {
        std::unique_ptr<JobFuncErasure> fn;
        {
            const std::lock_guard <std::mutex> lock(_mutex); //queue
            if (!_jobs.empty()) {
                fn = std::move(_jobs.front());
                _jobs.pop_front();
            }
        }
        if (fn) {
            (*fn.get())();
            return true;
        }
        return false;
    }

private:
    std::deque<std::unique_ptr<JobFuncErasure>> _jobs;
    std::mutex _mutex;
};

#endif //MC2TCPD_JOBQUEUE_H
