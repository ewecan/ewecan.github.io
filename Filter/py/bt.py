import bluetooth
devices = bluetooth.discover_devices(lookup_names=True)
# mac_address = [device[0] for device in devices if device[1] == 'Sichiray']
# print(mac_address)
# converted_mac = mac_address[0].replace(':', '')
# print(converted_mac)
print(devices)

