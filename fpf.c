#include <stdio.h>
#include <stdlib.h>

int main (int argc, char * argv[]){

	char *name = argv[1];
	printf("%s: Use \"%s [-i <index>] [-d <dev>] [<opts>]\n",
		name, name);
}
