#ifndef APP_SERVICE_H
#define APP_SERVICE_H

#include <stdint.h>
#include "proto_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

int app_service_init(void);
int app_service_handle_proto_msg(const proto_msg_t *msg);


#ifdef __cplusplus
}
#endif

#endif /* APP_SERVICE_H */
