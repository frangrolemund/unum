#include <stdio.h>

int main(int argc, char *argv[]) {
	printf("inside unum boot...\n");

	while (--argc) {
		printf("arg --> %s\n", *++argv);		
	}
}
