#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

int wifi_service_init(void);
int wifi_service_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_SERVICE_H */
