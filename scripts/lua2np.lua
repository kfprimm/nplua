#!/usr/bin/lua

function string.findlast(str, find)
	local i = str:len()
	while str:sub(i, i + (find:len() - 1)) ~= find do
		i = i - 1
	end
	return i, i + (find:len() - 1)
end

function NPObject(decl) end

function NPPlugin(opts)
	local desc, mime = "", ""
	for i,object in ipairs(opts['Objects']) do
		desc = desc.."|"..object[3]
		mime = mime.."|"..object[2]
	end
	desc = desc:sub(2, desc:len())
	mime = mime:sub(2, mime:len())

	io.output(io.open(opts['PluginName']..".def","w"))
	io.write([[
		LIBRARY "]]..opts['PluginName']..[[.dll"
		EXPORTS
		NP_GetEntryPoints
		NP_Initialize
		NP_Shutdown
	]])
	io.close()

	local DIR = os.getenv("LUA2NP_PATH")

	local template = ""
	local tmp = io.open(DIR.."/src/npapi.rc","r")
	template = tmp:read("*all")
	tmp:close()

	local rc = io.open(opts['PluginName']..".rc","w")
	rc:write([[
		#define PLUGIN_COMPANYNAME  "]]..opts['Author']..[["
		#define PLUGIN_DESCRIPTION  "]]..opts['Description']..[["
		#define PLUGIN_DESC         "]]..desc..[["
		#define PLUGIN_INTERNALNAME "]]..opts['PluginName']..[["
		#define PLUGIN_COPYRIGHT    "]]..opts['Copyright']..[["
		#define PLUGIN_MIME         "]]..mime..[["
		#define PLUGIN_FILENAME     "]]..opts['PluginName']..[[.dll"
		#define PLUGIN_NAME         "]]..opts['Name'].."\"",
	"\n", template)
	rc:close()

	os.execute("windres \""..opts['PluginName']..".rc\" \""..opts['PluginName']..".o\"")

	local script = debug.getinfo(1).source
	if os.getenv("OS") == "" then
		os.execute("gcc --shared -fPIC -Wall -Iinclude `pkg-config --cflags lua5.1` -o "..opts['PluginName']..".so src/npapi.c src/nplua.c `pkg-config --libs lua5.1`")
		os.execute("mv "..opts['PluginName']..".so ~/.mozilla/plugins")
	else
		os.execute("gcc -DWIN32 -std=c99 -shared -Wall -I"..DIR.."/include -I"..DIR.."/src/lua -o "..opts['PluginName']..".dll "..DIR.."/src/npapi.c "..DIR.."/src/nplua.c "..opts['PluginName']..".o "..DIR.."/lib/liblua51.a "..opts['PluginName']..".def "..opts['GCC_OPTS'])
		os.execute("cp "..opts['PluginName']..".dll "..os.getenv("APPDATA").."/Mozilla/plugins")
	end
end

dofile(arg[1])
