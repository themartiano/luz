#pragma once

#include <chrono>

class   Clock
{
	public:
		typedef std::chrono::steady_clock					ClockType;
		typedef std::chrono::duration<double>				Seconds;
		typedef std::chrono::duration<double, std::milli>	Milliseconds;
		typedef std::chrono::duration<double, std::micro>	Microseconds;

		Clock(void);
		void			start(void);
		Seconds			elapsedS(void);
		Milliseconds	elapsedMS(void);
		Microseconds	elapsedUS(void);

	private:
		ClockType::time_point	_startTime;
		bool					_running;
};
