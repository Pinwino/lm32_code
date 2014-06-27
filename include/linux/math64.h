#ifndef __MATH64_H
#define __MATH64_H

#include <sys/types.h>
#include <inttypes.h>
#include <linux/fls.h>

typedef uint64_t u64;
typedef uint32_t u32;

u64 div_u64_rem(u64 n, u32 base, u32 *remainder);
u64 div64_u64_rem(u64 div, u64 divisor, u64 *remainder);

static inline u64 div_u64(u64 dividend, u32 divisor)
{
	u32 remainder;
	return div_u64_rem(dividend, divisor, &remainder);
}

#endif /* __MATH64_H */

