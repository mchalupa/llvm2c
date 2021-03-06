#include <stdlib.h>

int main(int argc, char** argv) {
	if (argc != 3) {
		return -1;
	}

	char *p;
	long l = strtol(argv[1], &p, 10);

	if (*p != '\0') {
		return -1;
	}

	long r = strtol(argv[2], &p, 10);

	if (*p != '\0') {
		return -1;
	}

	if (l * l > r * r) {
		return l;
	}

	return r;
}
