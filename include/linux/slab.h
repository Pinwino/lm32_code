#ifndef __SLAB_H
#define __SLAB_H

#define kmalloc(a, b) 	malloc(a)
#define kfree(a)		free(a)

#define GFP_KERNEL

#endif /* __SLAB_H  */
