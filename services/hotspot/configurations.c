/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright 2011 Red Hat, Inc.
 */

/*
 * Compile with:
 *   gcc -Wall `pkg-config --libs --cflags glib-2.0 libnm` handler.c -o handler
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "hotspot.h"

/* Print details of connection */
void show_connection (NMConnection *connection)
{
	NMSettingConnection *s_con;
	guint64 timestamp;
	char *timestamp_str;
	char timestamp_real_str[64];
	const char *val1, *val2, *val3, *val4, *val5;

	s_con = nm_connection_get_setting_connection (connection);
	if (s_con) {
		/* Get various info from NMSettingConnection and show it */
		timestamp = nm_setting_connection_get_timestamp (s_con);
		timestamp_str = g_strdup_printf ("%" G_GUINT64_FORMAT, timestamp);
		strftime (timestamp_real_str, sizeof (timestamp_real_str), "%c", localtime ((time_t *) &timestamp));

		val1 = nm_setting_connection_get_id (s_con);
		val2 = nm_setting_connection_get_uuid (s_con);
		val3 = nm_setting_connection_get_connection_type (s_con);
		val4 = nm_connection_get_path (connection);
		val5 = timestamp ? timestamp_real_str : "never";

		printf ("%-25s | %s | %-15s | %-43s | %s\n", val1, val2, val3, val4, val5);

		g_free (timestamp_str);
	}
}
