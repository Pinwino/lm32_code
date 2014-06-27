#include <linux/math64.h>

u64 div_u64_rem(u64 n, u32 base, u32 *remainder){
	u64 rem = n;
	u64 b = base;
	u64 res, d = 1;
	u32 high = rem >> 32;
	
	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (u64) high << 32;
		rem -= (u64) (high*base) << 32;
	}
	
	while ((u64)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}
	
	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		
		b >>= 1;
		d >>= 1;
	} while (d);
	
	//mprintf("inside rem %llu\n", rem << 32);
	//mprintf("inside res %llu\n", res << 32);
	
	*remainder = rem;
		
	return res;
}

u64 div64_u64(u64 dividend, u64 divisor)
{
	u32 high = divisor >> 32;
	u64 quot;
	mprintf("%s\n", __func__);
	mprintf("%llu, ", dividend << 32);
	mprintf("%llu\n", divisor >> 32);
	
	if (high == 0) {
	quot = div_u64(dividend, divisor);
	} else {
		int n = 1 + fls(high);
		quot = div_u64(dividend >> n, divisor >> n);
		if (quot != 0)
			quot--;
		if ((dividend - quot * divisor) >= divisor)
			quot++;
	}
	mprintf("%s\n", __func__);
	mprintf("quot %llu\n", quot << 32);
	return quot;
}

u64 div64_u64_rem(u64 div, u64 divisor, u64 *remainder)
 {
	u32 high = divisor >> 32;
	u64 quot;
	u64 dividend = div; 
	mprintf("%s\n", __func__);
	mprintf("%llu, ", dividend << 32);
	mprintf("%llu, ", divisor >> 32);
	mprintf("%llu, ", divisor << 32);
	mprintf("%llu\n", *remainder << 32);
	
	if (high == 0) {
		mprintf("high\n");
		u32 rem32;
		quot = div_u64_rem(dividend, divisor, &rem32);
		*remainder = rem32;
	} else {
		mprintf("no high\n");

		int n = 1 + fls(high);
		quot = div_u64(dividend >> n, divisor >> n);

		if (quot != 0)
			quot--;
		*remainder = dividend - quot * divisor;
		if (*remainder >= divisor) {
			quot++;
		*remainder -= divisor;
		}
	}
	mprintf("%s\n", __func__);
//	mprintf("low quot %llu\n", quot << 32);
//	mprintf("high quot %llu\n", quot >> 32);
//	mprintf("high quot %llu\n", quot);

	mprintf("quot high %lu\n", (unsigned long)(quot >> 32));
	mprintf("quot low %lu\n", (unsigned long)(quot));	
	return quot;
}

u64 mul_u64(u64 arg1, u64 arg2)   
{   
	u64 result = 0;   
	u64 multiplier = arg2;  
	
	//mprintf("inside arg1 %llu\n", arg1 << 32);
	//mprintf("inside multiplier %llu\n", multiplier << 32);
	
	while (multiplier)   
		{   
			if (multiplier & 1)   
				{   
					result = result + arg1;   
                }   
			arg1 <<= 1;   
			multiplier >>= 1;   
			//mprintf("inside multiplier %llu\n", multiplier << 32);
        }   
	//mprintf("inside result %llu\n", result << 32);
	return result;   
}   
