#include <Converter.h>

#include <memory>

int main(int argc, char* argv[])
{
	auto m = std::make_unique<Converter>();
	return m->run(argc, argv);
}