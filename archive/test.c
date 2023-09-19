#include <unistd.h>
#include <stdlib.h>

int main(void)
{
	unsigned int a = 237;
	unsigned int b = 149;
	unsigned int c = 156;
	write(1, &a, 1);
	write(1, &b, 1);
	sleep(5);
	// exit(1);
	write(1, &c, 1);
	return 0;
}
