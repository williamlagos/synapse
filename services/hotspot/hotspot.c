#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hotspot.h"

void start (int max, char **buffer)
{
	NMClient *client;
	GError *error = NULL;
	const GPtrArray *connections;
	const GPtrArray *devices;
	const GPtrArray *aps;
	int d,i,j,k;
	int connected = NOT_CONNECTED;

	d = 0;
	/* Verify if file lock exists */
	if (access("/tmp/hotspot.lock",F_OK) == 0) {
		fprintf(stdout,"Warning: Hotspot is locked. If isn't, remove /tmp/hotspot.lock.\n");
		exit(EXIT_FAILURE);
	}

	if (!(client = nm_client_new (NULL, &error))) {
		g_message ("Error: Could not connect to NetworkManager: %s.", error->message);
		g_error_free (error);
		exit(EXIT_FAILURE);
	}

	/* Now the connections can be listed. */
	connections = nm_client_get_connections (client);

	/* Get all devices managed by NetworkManager */
	devices = nm_client_get_devices (client);

	/* Go through the array and process Wi-Fi devices */
	for (i = 0; i < devices->len; i++) {
		NMDevice *device = g_ptr_array_index (devices, i);
		if (NM_IS_DEVICE_WIFI (device)) {
			d++;
			aps = nm_device_wifi_get_access_points (NM_DEVICE_WIFI (device));
			for (j = 0; j < aps->len; j++) {
				NMAccessPoint *ap = g_ptr_array_index (aps, j);
				GBytes *ssid = nm_access_point_get_ssid(ap);
				char *ssid_str = nm_utils_ssid_to_utf8 (g_bytes_get_data (ssid, NULL), g_bytes_get_size (ssid));
				// fprintf(stdout,"%s\n",ssid_str);
				for (k = 0; k < connections->len; k++) {
					NMSettingConnection *s_con = nm_connection_get_setting_connection (connections->pdata[k]);
					const char *ssid_con = nm_setting_connection_get_id (s_con);
					// fprintf(stdout,"%s\n",ssid_con);
					if(strcmp(ssid_str,ssid_con) == 0){
						connected = CONNECTED;
						// fputs("Found!\n",stdout);
					}
				}
			}
		}
	}

	g_object_unref (client);

	// Verify if there is any known connection
	if(!connected){
		// If not, start Hotspot service
		system("systemctl stop NetworkManager");
		system("systemctl start create_ap");
		// fputs("Start Hotspot\n",stdout);
		/* pid_t hotspot_id = fork();
		if(hotspot_id == 0) system("create_ap --no-virt wlan0 eth0 Hubspot hub12345");
		else {
			FILE *f = fopen("/tmp/hotspot.lock","w");
			fprintf(f,"%d",hotspot_id);
			fprintf(stdout,"PID: %d",hotspot_id);
			fclose(f);
			exit(EXIT_SUCCESS);
		} */
	}
}

int main(int argc, char** argv)
{
	start(argc,argv);
	return EXIT_SUCCESS;
}
