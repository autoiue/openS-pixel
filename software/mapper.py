from socket import *
from struct import *
import time
import threading

clients_color = {}
clients_rssi = {}
clients = {}
zones_color = {}
direct_color = {}

client_index = 0

def load_clients():

	global client_index

	with open('map.txt') as f:
		for mac in f:
			mac = mac[:-1]
			clients[mac] = client_index
			client_index = client_index + 4
	pass

load_clients()

def add_client(mac):

	global client_index

	if mac not in clients:
		clients[mac] = client_index
		client_index = client_index + 4
		with open('map.txt', 'a') as f:
			f.write(mac+"\r")		
	pass

# arduino map
def inomap(x, in_min, in_max, out_min, out_max):
	x = min(in_max, max(in_min, x))
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

# 4 bytes int color from 4 ints
def tocolor(r, g, b, w):
	return w << 24 | (r & 0xFF) << 16 | (g & 0xFF) << 8 | (b & 0xFF)

def listenShine():
	Shine = socket(AF_INET, SOCK_DGRAM)
	Shine.bind(('', 1935))
	while True:

		# receive message from Shine where 	0x0 is universe
		#									0x1 is address
		#									0x2 is value

		message, address = Shine.recvfrom(32)

		# ZONED universe == 10
		if message[0] == 10:
			zones_color[message[1]] = message[2]
			pass
		# DIRECT universe == 11
		elif message[0] == 11:
			direct_color[message[1]] = message[2]
			pass
		#mac = ':'.join( [ "%02X" % x for x in message ] )
		#print("Shine >\t", mac)
	pass

def listenRouter():
	Router = socket(AF_INET, SOCK_DGRAM)
	Router.bind(('', 1934))
	while True:
		# receive message from router where 0x00 > 0x10 is ascii MAC address
		# 									0x12 > 0x14 is ascii RSSI
		message, address = Router.recvfrom(32)
		# read MAC
		mac = message[:-4].decode("ascii")
		# read RSSI
		signal = int(message[-2:])
		# map RSSI from 0 to 49
		clients_rssi[mac] = signal

def listenESP():

	global client_index

	ESPs = socket(AF_INET, SOCK_DGRAM)
	ESPs.bind(('', 1933))
	while True:
		
		# receive mac address from client
		message, address = ESPs.recvfrom(32)
		mac = ':'.join( [ "%02X" % x for x in message ] )

		# set client no.
		if mac not in clients:
			add_client(mac)

		# no. of the client
		index = clients[mac]

		# defaults
		r = 0
		g = 0
		b = 0
		w = 0

		# if in direct color
		if index + 3 in direct_color:
			r = direct_color[index]
			g = direct_color[index+1]
			b = direct_color[index+2]
			w = direct_color[index+3]

		# if we have zone info for the device, and this zone has color info
		if mac in clients_rssi:
			zone = round(inomap(clients_rssi[mac], 25, 65, 0, 24))
			if zone*4+3 in zones_color:
				zone = zone*4
				# blend direct color + zoned color
				r = min(255, r + zones_color[zone])
				g = min(255, g + zones_color[zone+1])
				b = min(255, b + zones_color[zone+2])
				w = min(255, w + zones_color[zone+3])

		# link color to client and send it
		clients_color[mac] = tocolor(r, g, b, w)
		ESPs.sendto(pack('!IB', clients_color[mac], 0), address)


ESP_server = threading.Thread(target=listenESP)
ESP_server.start()
Router_server = threading.Thread(target=listenRouter)
Router_server.start()
Shine_server = threading.Thread(target=listenShine)
Shine_server.start()

while False:
	print("mapper.py")
	print()
	print("Zones: "+str(len(zones_color))+"("+str(len(zones_color)//4)+")\t", "Direct: "+str(len(direct_color))+"("+str(len(direct_color)//4)+")\t", )
	print("MAC address:", "\t\t", "dBm", "\t", " ID" "\t", "color:")
	for mac in clients_rssi:
		if mac in clients_color:
			print(mac, '\t', "-"+str(clients_rssi[mac])+"("+str(round(inomap(clients_rssi[mac], 25, 65, 0, 24)))+")", ' ', clients[mac]//4, '\t', "0x%0.8X" % clients_color[mac])
		else:
			print(mac, '\t', clients_rssi[mac], '\t', '/')
		pass

	#print(direct_color)
	#print(zones_color)
	
	time.sleep(1)
	print("\033[H\033[J")
	
