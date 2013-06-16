#include "singtest.hpp"

namespace mapnik {

bool registered = singtest::instance().createStatement();

singtest::singtest()
{
	std::cout << "singtest CREATED." << std::endl;
}

singtest::~singtest()
{
	std::cout << "singtest destroyed." << std::endl;
}

bool singtest::createStatement()
{
	std::cout << "singtest statement." << std::endl;
	return true;
}

}