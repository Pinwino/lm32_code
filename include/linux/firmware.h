#ifndef __FIRMWARE_H
#define __FIRMWARE_H
#include <inttypes.h>
#include <sys/types.h>

struct firmware {
	size_t size;
	void *data;
};

#define request_firmware(a, b, c)	1
#define release_firmware(a) 		;

#endif /* __FIRMWARE_H */
