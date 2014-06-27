#ifndef __KEREL_2_LM32_H
#define __KEREL_2_LM32_H

#define dev_err(NULL, ...) 	kernel_dev(0, __VA_ARGS__)
#define dev_warn(NULL, ...) kernel_dev(1, __VA_ARGS__)
#define dev_info(NULL, ...) kernel_dev(2, __VA_ARGS__)

#define pr_err(...)		kernel_dev(0, __VA_ARGS__)
#define pr_warn(...)	kernel_dev(1, __VA_ARGS__)
#define pr_debug(...)	kernel_dev(1, __VA_ARGS__)

#define be64_to_cpu(a) a
#define be32_to_cpu(a) a
#define be16_to_cpu(a) a

#define in_atomic() 0
#define printk pp_printf
#define printf pp_printf

#define fprintf(a,  ...) kernel_dev(0xfff, __VA_ARGS__)
#define fprintf(a,  ...) kernel_dev(0xfff, __VA_ARGS__)


#endif /* __KEREL_2_LM32_H */
