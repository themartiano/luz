#include "Clock.hpp"

/*
	Constructors
*/

// Constructs the Clock
Clock::Clock(void)
{
	this->_running = false;
	this->_startTime = ClockType::time_point(); // Epoch, 0
}

// Starts (or restarts) the Clock
void	Clock::start(void)
{
	this->_running = true;
	this->_startTime = ClockType::now();
}

// Returns elapsed seconds (s) since start
double	 Clock::elapsedS(void)
{
	if (this->_running == true)
	{
		return (
			std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(ClockType::now() - this->_startTime).count()
		);
	}

	return (0.0);
}

// Returns elapsed milliseconds (ms) since start
double	 Clock::elapsedMS(void)
{
	if (this->_running == true)
	{
		return (
			std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(ClockType::now() - this->_startTime).count()
		);
	}

	return (0.0);
}

// Returns elapsed microseconds (μs) since start
double	 Clock::elapsedUS(void)
{
	if (this->_running == true)
	{
		return (
			std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(ClockType::now() - this->_startTime).count()
		);
	}

	return (0.0);
}
