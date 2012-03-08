#!/usr/bin/lua

function string.findlast(str, find)
	if str:find(find) ~= nil then
		local i = str:len()
		while str:sub(i, i + (find:len() - 1)) ~= find do
			i = i - 1
		end
		return i, i + (find:len() - 1)
	end
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

	local DIR = arg[0]:sub(1, arg[0]:findlast("\\"))..".."
	local SCRIPT_FILE = "__lua_script.c"

	io.output(io.open(SCRIPT_FILE,"w"))
	io.write([[
		#include <nplua.h>
		int nplua_execute(lua_State *L)
	]])
	io.close()
	os.execute("lua "..DIR.."/scripts/bin2c.lua "..arg[1].." >> "..SCRIPT_FILE)

	local C_OPTS, EXT = "-DDEBUG -m32 -std=c99 -shared -Wall -I"..DIR.."/include -I"..DIR.."/src/lua", ""

	if os.getenv("OS") == nil then
		EXT = "so"
		opts['GCC_OPTS'] = (opts['GCC_OPTS'] or  "").." -lm"
	else
		EXT = "dll"
		C_OPTS = C_OPTS.." -DWIN32"
		opts['GCC_OPTS'] = (opts['GCC_OPTS'] or  "").." "..opts['PluginName']..".def "..opts['PluginName']..".o"

		io.output(io.open(opts['PluginName']..".def","w"))
		io.write([[
			LIBRARY "]]..opts['PluginName']..[[.dll"
			EXPORTS
			NP_GetEntryPoints
			NP_Initialize
			NP_Shutdown
		]])
		io.close()

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
	end

	os.execute("gcc "..C_OPTS.." -o  "..opts['PluginName'].."."..EXT.." "..DIR.."/src/npapi.c "..DIR.."/src/nplua.c "..opts['GCC_OPTS'].." "..SCRIPT_FILE.." "..DIR.."/lib/liblua51.a")
	os.execute("rm -rf "..SCRIPT_FILE)
end

dofile(arg[1])
