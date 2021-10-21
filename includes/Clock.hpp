#ifndef CLOCK_HPP
# define CLOCK_HPP

#include <chrono>

class   Clock
{
    public:
        Clock(void);
        Clock(bool startNow);
        void    start(void);
        double  elapsed(void);
        double  stop(void);

    private:
        std::chrono::steady_clock::time_point   _startTimeMS;
        bool                                    _running;
};

#endif