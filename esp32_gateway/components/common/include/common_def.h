#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RET_OK               (0)
#define RET_ERR              (-1)
#define RET_NULL_PTR         (-2)
#define RET_INVALID_PARAM    (-3)
#define RET_CHECKSUM_ERR     (-4)
#define RET_PROTO_ERR        (-5)

#ifdef __cplusplus
}
#endif

#endif /* COMMON_DEF_H */