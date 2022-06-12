#pragma once

#include <string>

class	Font
{
	public:
		Font(const std::string& path);

	private:
		void	_read(const std::string& path);
};
