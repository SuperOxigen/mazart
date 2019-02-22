#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool bool_t;

#ifdef __GNUC__
#define __unused __attribute__((unused))
#else /* no __GNUC__ */
#define __unused
#endif

#endif /* _COMMON_H_ */
