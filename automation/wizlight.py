# {'ip': '192.168.0.11', 'port': 38899, 'state': None, 'mac': 'a8bb50afcbc1', 'bulbtype': None, 'whiteRange': None, 'extwhiteRange': None} - Doorway
# {'ip': '192.168.0.12', 'port': 38899, 'state': None, 'mac': 'a8bb50afcbbc', 'bulbtype': None, 'whiteRange': None, 'extwhiteRange': None} - Study Desk
# {'ip': '192.168.0.13', 'port': 38899, 'state': None, 'mac': 'a8bb50b99d0c', 'bulbtype': None, 'whiteRange': None, 'extwhiteRange': None} - TV Bulb
# {'ip': '192.168.0.14', 'port': 38899, 'state': None, 'mac': 'a8bb50fa6bf3', 'bulbtype': None, 'whiteRange': None, 'extwhiteRange': None} - Doorway Bulb
# {'ip': '192.168.0.15', 'port': 38899, 'state': None, 'mac': 'a8bb50b0ca6f', 'bulbtype': None, 'whiteRange': None, 'extwhiteRange': None} - Main 

import asyncio, sys

from pywizlight import wizlight, PilotBuilder, discovery

bulbs = []

async def init(broadcast_ip="192.168.0.255"):
	global bulbs
	bulbs = await discovery.discover_lights(broadcast_space=broadcast_ip)
	for bulb in bulbs:
		print(bulb.ip)

async def set(brightness, temp=2700):
	global bulbs
	if len(bulbs)==0:
		await init()
	timeout = 10
	for bulb in bulbs:
		# print(bulb.__dict__)
		if bulb.ip=='192.168.0.12':
			# await bulb.turn_on(PilotBuilder(brightness=brightness, colortemp=temp))
		# if brightness == 0:
			await bulb.turn_off()
		# else:
		# 	await asyncio.wait_for(bulb.turn_on(PilotBuilder(brightness=brightness, colortemp=temp)), timeout)
			# await asyncio.wait_for(bulb.turn_on(PilotBuilder(scene=45)), timeout)

async def main():
	"""Sample code to work with bulbs."""
	# Discover all bulbs in the network via broadcast datagram (UDP)
	# function takes the discovery object and returns a list with wizlight objects.
	bulbs = await discovery.discover_lights(broadcast_space="192.168.0.255")
	# Print the IP address of the bulb on index 0
	# print(f"Bulb IP address: {bulbs[0].ip}")

	# print(len(bulbs))

	# Iterate over all returned bulbs
	# for bulb in bulbs:
	# 	print(bulb.__dict__)
		# Turn off all available bulbs
		# await bulb.turn_off()

	# light = wizlight(bulb[0].ip, bulb[0].port)
	# light = bulbs[0]

	# Set up a standard light
	# light = wizlight("192.168.1.27")
	# # Set up the light with a custom port
	# #light = wizlight("your bulb's IP address", port=12345)

	# # The following calls need to be done inside an asyncio coroutine
	# # to run them fron normal synchronous code, you can wrap them with
	# # asyncio.run(..).

	# # Turn on the light into "rhythm mode"
	# await light.turn_on(PilotBuilder())
	# # Set bulb brightness
	# await light.turn_on(PilotBuilder(brightness = 255))

	# # Set bulb brightness (with async timeout)
	# timeout = 10
	# for i in range(0,33):
	# 	try:
	# 		await asyncio.wait_for(light.turn_on(PilotBuilder(scene=i)), timeout)
	# 	except:
	# 		continue
	# 	print(i)
		# await asyncio.sleep(5)
	# await asyncio.wait_for(light.turn_on(PilotBuilder(colortemp = 6500)), timeout)

	# # Set bulb to warm white
	# await light.turn_on(PilotBuilder(warm_white = 255))

	# # Set RGB values
	# # red to 0 = 0%, green to 128 = 50%, blue to 255 = 100%
	# await light.turn_on(PilotBuilder(rgb = (0, 128, 255)))

	# # Get the current color temperature, RGB values
	# state = await light.updateState()
	# print(state.get_colortemp())
	# red, green, blue = state.get_rgb()
	# print(f"red {red}, green {green}, blue {blue}")

	# # Start a scene
	# await light.turn_on(PilotBuilder(scene = 4)) # party

	# # Get the name of the current scene
	# state = await light.updateState()
	# print(state.get_scene())

	# # Get the features of the bulb
	# bulb_type = await bulbs[0].get_bulbtype()
	# print(bulb_type.features.brightness) # returns true if brightness is supported
	# print(bulb_type.features.color) # returns true if color is supported
	# print(bulb_type.features.color_tmp) # returns true if color temperatures are supported
	# print(bulb_type.features.effect) # returns true if effects are supported
	# print(bulb_type.kelvin_range.max) # returns max kelvin in in INT
	# print(bulb_type.kelvin_range.min) # returns min kelvin in in INT
	# print(bulb_type.name) # returns the module name of the bulb

	# Turns the light off
	# await light.turn_off()

	# Do operations on multiple lights parallely
	#bulb1 = wizlight("<your bulb1 ip>")
	#bulb2 = wizlight("<your bulb2 ip>")
	#await asyncio.gather(bulb1.turn_on(PilotBuilder(brightness = 255)),
	   # bulb2.turn_on(PilotBuilder(warm_white = 255)), loop = loop)

loop = asyncio.get_event_loop()
args = map(int, sys.argv[1:])
loop.run_until_complete(set(*args))