#ifndef __KERNEL_ADAPT_H__
#define __KERNEL_ADAPT_H__


#define dev_err(...) kernel_dev(0, __VA_ARGS__)
#define dev_info(...) kernel_dev(0, __VA_ARGS__)

#endif
