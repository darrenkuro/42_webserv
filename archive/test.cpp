#include <iostream>

int main(void)
{
	std::string root = "/abc/";
	std::string path = "def";
    root = root[root.size() - 1] == '/' ? root.substr(0, root.size() - 1) : root;
    path = path[0] == '/' ? path.substr(1) : path;
    std::cout << root + "/" + path << std::endl;
	return 0;
}
