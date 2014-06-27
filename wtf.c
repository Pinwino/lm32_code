#include <stdio.h>
#include <inttypes.h>

int main (int argc, char *argv[]){
	uint64_t picoseconds = 150, utc = 278;
	
	printf ("%10llu:%03llu,%03llu,%03llu,%03llu ps\n",
			(long long)(utc),
			(picoseconds / (1000LL * 1000 * 1000)),
			(picoseconds / (1000LL * 1000) % 1000),
			(picoseconds / (1000LL) % 1000),
			(picoseconds % 1000LL));
	
	return 0;
}
