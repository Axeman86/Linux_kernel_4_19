#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int fd = open("/dev/second", O_RDONLY);
	if (fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	int counter = 0;
	int old_counter = 0;
	while (1) {
		read(fd, &counter, sizeof(counter));
		if (counter != old_counter) {
			printf("second after open /dev/second: %d\n", counter);
			old_counter = counter;
		}
	}

	return EXIT_SUCCESS;
}
