#!/usr/bin/lua

require "iwinfo"
local socket = require("socket")

local device = "wlan0-1"
local server = "192.168.2.10"
local port = 1934

local udp = assert(socket.udp())
assert(udp:setpeername(server, port))

function getinfo(ifname, func)
	local driver_type = iwinfo.type(ifname)
	if driver_type and iwinfo[driver_type][func] then
		return iwinfo[driver_type][func](ifname)
	end

	return nil
end

function mactobytes()
end
    
function extract()
	local opmode = getinfo(device, "mode")
	if opmode then	
		local assoclist = getinfo(device, "assoclist")
		if assoclist then
			local mac, info
			for mac, info in pairs(assoclist) do
				--mac = split(mac, ":")
				udp:send(mac .. "." .. info.signal)
				--print("Client #" .. mac .. ": " .. info.signal .. "dB")
			end

			--print("There are " .. count .. " clients")
		else
			print("Unable to fetch associated client list")
		end
	else
		print("Device " .. device .. " appears to be down")
	end
end

while true do
	extract()
	socket.sleep(0.1)
end