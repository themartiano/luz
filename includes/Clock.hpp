#pragma once

#include <chrono>

class   Clock
{
	public:
		typedef std::chrono::steady_clock	ClockType;

		Clock(void);
		void	start(void);
		double	elapsedS(void);
		double	elapsedMS(void);
		double	elapsedUS(void);

	private:
		ClockType::time_point	_startTime;
		bool					_running;
};
