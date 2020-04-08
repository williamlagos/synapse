#ifndef HOTSPOT_H
#define HOTSPOT_H

#define NOT_CONNECTED 0
#define CONNECTED 1

#include <NetworkManager.h>

void show_connection (NMConnection *connection);
char *ap_wpa_rsn_flags_to_string (guint32 flags);
void show_access_point_info (NMAccessPoint *ap);
void show_wifi_device_info (NMDevice *device);

#endif
