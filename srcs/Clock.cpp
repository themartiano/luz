#include "Clock.hpp"

/*
	Constructors
*/

// Constructs the Clock and starts counting
Clock::Clock(void)
{
    this->start();
}

// Constructs the Clock and starts counting if 'startNow' is true.
Clock::Clock(bool startNow)
{
    if (startNow == true)
    {
        this->start();
    }
    else
    {
        this->_running = false;
    }
}

// Starts (or restarts) the Clock
void    Clock::start(void)
{
    this->_running = true;
    this->_startTimeMS = std::chrono::steady_clock::now();
}

// Returns elapsed seconds since start
double     Clock::elapsed(void)
{
    if (this->_running == true)
    {
        double elapsedMS = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - this->_startTimeMS).count();
        return (elapsedMS / 1000.0);
    }
    return (0.0);
}

// Returns elapsed seconds since start and stops the Clock
double     Clock::stop(void)
{
    if (this->_running == true)
    {
        double elapsedS = this->elapsed();
        this->_running = false;
        return (elapsedS);
    }
    return (0.0);
}
