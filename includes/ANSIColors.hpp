#ifndef ANSICOLORS_H
# define ANSICOLORS_H

#include <string>

// Reset
const std::string CLR_RESET("\033[0m");

// Regular Colors
const std::string CLR_BLACK("\033[0;30m");
const std::string CLR_RED("\033[0;31m");
const std::string CLR_GREEN("\033[0;32m");
const std::string CLR_YELLOW("\033[0;33m");
const std::string CLR_BLUE("\033[0;34m");
const std::string CLR_PURPLE("\033[0;35m");
const std::string CLR_CYAN("\033[0;36m");
const std::string CLR_WHITE("\033[0;37m");

// Bold
const std::string CLR_BLACK_BOLD("\033[1;30m");
const std::string CLR_RED_BOLD("\033[1;31m");
const std::string CLR_GREEN_BOLD("\033[1;32m");
const std::string CLR_YELLOW_BOLD("\033[1;33m");
const std::string CLR_BLUE_BOLD("\033[1;34m");
const std::string CLR_PURPLE_BOLD("\033[1;35m");
const std::string CLR_CYAN_BOLD("\033[1;36m");
const std::string CLR_WHITE_BOLD("\033[1;37m");

// Underline
const std::string CLR_BLACK_UNDERLINED("\033[4;30m");
const std::string CLR_RED_UNDERLINED("\033[4;31m");
const std::string CLR_GREEN_UNDERLINED("\033[4;32m");
const std::string CLR_YELLOW_UNDERLINED("\033[4;33m");
const std::string CLR_BLUE_UNDERLINED("\033[4;34m");
const std::string CLR_PURPLE_UNDERLINED("\033[4;35m");
const std::string CLR_CYAN_UNDERLINED("\033[4;36m");
const std::string CLR_WHITE_UNDERLINED("\033[4;37m");

// Background
const std::string CLR_BLACK_BK("\033[40m");
const std::string CLR_RED_BK("\033[41m");
const std::string CLR_GREEN_BK("\033[42m");
const std::string CLR_YELLOW_BK("\033[43m");
const std::string CLR_BLUE_BK("\033[44m");
const std::string CLR_PURPLE_BK("\033[45m");
const std::string CLR_CYAN_BK("\033[46m");
const std::string CLR_WHITE_BK("\033[47m");

// High Intensity
const std::string CLR_BLACK_BRIGHT("\033[0;90m");
const std::string CLR_RED_BRIGHT("\033[0;91m");
const std::string CLR_GREEN_BRIGHT("\033[0;92m");
const std::string CLR_YELLOW_BRIGHT("\033[0;93m");
const std::string CLR_BLUE_BRIGHT("\033[0;94m");
const std::string CLR_PURPLE_BRIGHT("\033[0;95m");
const std::string CLR_CYAN_BRIGHT("\033[0;96m");
const std::string CLR_WHITE_BRIGHT("\033[0;97m");

// Bold High Intensity
const std::string CLR_BLACK_BOLD_BRIGHT("\033[1;90m");
const std::string CLR_RED_BOLD_BRIGHT("\033[1;91m");
const std::string CLR_GREEN_BOLD_BRIGHT("\033[1;92m");
const std::string CLR_YELLOW_BOLD_BRIGHT("\033[1;93m");
const std::string CLR_BLUE_BOLD_BRIGHT("\033[1;94m");
const std::string CLR_PURPLE_BOLD_BRIGHT("\033[1;95m");
const std::string CLR_CYAN_BOLD_BRIGHT("\033[1;96m");
const std::string CLR_WHITE_BOLD_BRIGHT("\033[1;97m");

// High Intensity backgrounds
const std::string CLR_BLACK_BK_BRIGHT("\033[0;100m");
const std::string CLR_RED_BK_BRIGHT("\033[0;101m");
const std::string CLR_GREEN_BK_BRIGHT("\033[0;102m");
const std::string CLR_YELLOW_BK_BRIGHT("\033[0;103m");
const std::string CLR_BLUE_BK_BRIGHT("\033[0;104m");
const std::string CLR_PURPLE_BK_BRIGHT("\033[0;105m");
const std::string CLR_CYAN_BK_BRIGHT("\033[0;106m");
const std::string CLR_WHITE_BK_BRIGHT("\033[0;107m");

#endif