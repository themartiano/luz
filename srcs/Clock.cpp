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
Clock::Seconds	 Clock::elapsedS(void)
{
	if (this->_running == true)
	{
		return (
			Clock::Seconds(ClockType::now() - this->_startTime)
		);
	}

	return (Clock::Seconds(0.0));
}

// Returns elapsed milliseconds (ms) since start
Clock::Milliseconds	 Clock::elapsedMS(void)
{
	if (this->_running == true)
	{
		return (
			Clock::Milliseconds(ClockType::now() - this->_startTime)
		);
	}

	return (Clock::Milliseconds(0.0));
}

// Returns elapsed microseconds (μs) since start
Clock::Microseconds	 Clock::elapsedUS(void)
{
	if (this->_running == true)
	{
		return (
			Clock::Microseconds(ClockType::now() - this->_startTime)
		);
	}

	return (Clock::Microseconds(0.0));
}
