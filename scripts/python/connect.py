"""
Add a connection to NetworkManager. You do this by sending a dict to
AddConnection. The dict below was generated with n-m dump on an existing
connection and then anonymised
"""

import NetworkManager
import uuid

example_connection = {
'802-11-wireless': {
	'security': '802-11-wireless-security', 
	'ssid': 'FabricaDeIdeias', 
	'name': '802-11-wireless', 'mode': 'infrastructure'}, 
	'connection': {
		'autoconnect': True, 
		'type': '802-11-wireless', 
		'id': 'FabricaDeIdeias', 
		'name': 'connection', 
		'uuid': '0e899218-3d0d-11e5-a03b-b827ebc7ab94'
	}, 
	'ipv6': {
		'method': 'ignore', 
		'name': 'ipv6'
	}, 
	'ipv4': {
		'method': 'auto', 
		'name': 'ipv4'
	}, 
	'802-11-wireless-security': {
		'key-mgmt': 'wpa-psk', 
		'name': '802-11-wireless-security', 
		'psk': 'sejabemvindo'
	}
}

NetworkManager.Settings.AddConnection(example_connection)
