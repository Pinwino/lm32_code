#include <stdio.h>
#include <pp-printf.h>

int main(int argc, char **argv)
{
	pp_printf("integer %5i %5i %05i\n", 1024, 666, 53);
	pp_printf("octal   %5o %5o %05o\n", 1024, 666, 53);
	pp_printf("hex     %5x %5x %05x\n", 1024, 666, 53);
	pp_printf("HEX etc %5X %+5d %-5i\n", 1024, 666, 53);
	pp_printf("char: %c  string %s %5s %.5s\n", 65, "foo", "foo",
		  "verylongstring");
	return 0;
}
