#include "commons.hpp"

std::string Utils::toString(float f, int width, int precision, bool left) {
	std::ostringstream temp;
	if(left) temp << std::left;
	temp << std::fixed << std::setprecision(precision) << std::setw(width) << f;
	return temp.str();
}
