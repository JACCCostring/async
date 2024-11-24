#ifndef __ASYNC__

#define __ASYNC__

#include <condition_variable>
#include <functional>
#include <exception>
#include <thread>
#include <future>
#include <chrono>
#include <queue>
#include <mutex>

/*
class can be inherit and will work similar as it was not inherit,
but class pimpl_type must be re-implemented as a template spetialization
of the desired type ex: template<>
                        class async::AsyncTask<datatype>::pimpl_type
                        {
                            public:

                            private:
                        };

Note:
    datatype primitive are ok but, if a complex datatype is provided then,
    make sure (!=) or (different then) operator is implemented in the type plus
    default and parameterized constructors and T default_value() method implemented
*/

namespace async
{
    template <typename T>
    class AsyncTask
    {
    public:
        using ValueType = T;
        using ReturnType = T;

        using Sender_callback = std::function<ReturnType()>;
        using Receiver_callback = std::function<void(ValueType)>;

        AsyncTask()
        {
            _default_value = _pimpl_t->default_value();
        };

        ~AsyncTask()
        {
            _return_value = _default_value;

            if (_loop_thread.joinable())
                _loop_thread.join();
        }

        void run()
        {
            if (_senders.empty() or !_on_receiving)
                throw std::runtime_error("can not run an instance that has not receiver and assigned task !");

            _loop();
        }

        void set_sender(const Sender_callback &s)
        {
            std::scoped_lock<std::mutex> lock{_mutex};
            // code here
            auto future = std::async(std::launch::async, s);

            _senders.push(std::move(future));

            _return_value = _default_value;

            _condition_var.notify_one();
        }

        void on_receive(Receiver_callback receiver)
        {
            std::scoped_lock<std::mutex> lock{_mutex};

            // code here
            _on_receiving = receiver;
        }

    private: // private section for methods
        void _logic_loop()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock{_mutex};

                _condition_var.wait(lock, [&]
                                    { 
                    auto sender = std::move(_senders.front());

                    _senders.pop();

                    _return_value = sender.get();

                    return _return_value != _default_value; });

                // code here
                _on_receiving(_return_value);

                _return_value = _default_value;

                if (_senders.empty())
                    break;
            }
        }

        void _loop()
        {
            std::scoped_lock<std::mutex> lock{_mutex};

            if (!_loop_thread.joinable())
                _loop_thread = std::thread(&AsyncTask::_logic_loop, this);
        }

    private:
        std::mutex _mutex;                      // mutex
        std::thread _loop_thread;               // thread for loop
        std::condition_variable _condition_var; // to know when signal or not

        std::queue<std::future<ReturnType>> _senders; // collection of futures to ReturnType = T

        Receiver_callback _on_receiving; // std::function<> for setting receiver callback

        ValueType _default_value;
        ReturnType _return_value;

        // pimpl class for user to reimplement before using this class
        class pimpl_type;

        // unique smart pointer to pimpl_type class for better memory management
        std::unique_ptr<pimpl_type> _pimpl_t;
    };
}

#endif