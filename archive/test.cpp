#include <iostream>

int main(int ac, char **av)
{
	if (ac != 2)
		return 1;
	std::string str = std::string(av[1]);
	size_t endPos = str.find_last_of('/');
	std::string test = endPos == 0 ? "/" : str.substr(0, str.find_last_of('/'));
	std::cout << test << std::endl;
	return 0;
}
