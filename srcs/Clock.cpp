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
void	Clock::start(void)
{
	this->_running = true;
	this->_startTimeMS = std::chrono::steady_clock::now();
}

// Returns elapsed seconds since start
double	 Clock::elapsed(bool seconds)
{
	if (this->_running == true)
	{
		double elapsedMS = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - this->_startTimeMS).count();
		if (seconds)
		{
			return (elapsedMS / 1000.0);
		}
		else
		{
			return (elapsedMS);
		}
	}
	return (0.0);
}

// Returns elapsed seconds since start
double	 Clock::elapsed(void)
{
	return (this->elapsed(true));
}

// Returns elapsed seconds since start and stops the Clock
double	 Clock::stop(bool seconds)
{
	if (this->_running == true)
	{
		double elapsedS = this->elapsed(seconds);
		this->_running = false;
		return (elapsedS);
	}
	return (0.0);
}

// Returns elapsed seconds since start and stops the Clock
double	 Clock::stop(void)
{
	return (this->stop(true));
}

