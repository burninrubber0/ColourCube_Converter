#pragma once

#include <string>

class Converter
{
	std::string input;
	std::string output;

	void showHelp();
	int getArgs(int argc, char* argv[]);
	int convert();

public:
	Converter();

	int run(int argc, char* argv[]);
};