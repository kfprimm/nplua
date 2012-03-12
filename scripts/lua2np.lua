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

function cmd(c, verbose)
	if verbose then print(c) end
	assert( os.execute(c) == 0 )
end

function NPObject(decl) end

function NPPlugin(opts)
	local verbose = true
	local desc, mime = "", ""
	for i,object in ipairs(opts['Objects']) do
		desc = desc.."|"..object[3]
		mime = mime.."|"..object[2]
	end
	desc = desc:sub(2, desc:len())
	mime = mime:sub(2, mime:len())

	local DIR = arg[0]:gsub("\\", "/"):sub(1, arg[0]:findlast("/"))..".."
	local SCRIPT_FILE = "__lua_script.c"

	io.output(io.open(SCRIPT_FILE,"w"))
	io.write([[
		#include <nplua.h>
		int nplua_execute(lua_State *L)
	]])
	io.close()
	cmd("lua "..DIR.."/scripts/bin2c.lua "..arg[1].." >> "..SCRIPT_FILE, verbose)

	table.remove(arg, 1)
	local GCC_OPTS = table.concat(arg, " ") or ""

	local C_OPTS, EXT = "-DDEBUG -m32 -std=c99 -shared -Wall -I"..DIR.."/include", ""

	if os.getenv("OS") == nil then
		EXT = "so"
		C_OPTS = C_OPTS.." `pkg-config lua5.1 --cflags`"
		GCC_OPTS = GCC_OPTS.." -lm `pkg-config lua5.1 --libs`"
	else
		EXT = "dll"
		C_OPTS = C_OPTS.." -DWIN32"
		GCC_OPTS = GCC_OPTS.." -I"..DIR.."/src/lua "..opts['PluginName']..".def "..opts['PluginName']..".o "..DIR.."/lib/liblua51.a"

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

		cmd("windres \""..opts['PluginName']..".rc\" \""..opts['PluginName']..".o\"", verbose)
	end

	cmd("gcc "..C_OPTS.." -o  "..opts['PluginName'].."."..EXT.." "..DIR.."/src/npapi.c "..DIR.."/src/nplua.c "..SCRIPT_FILE.." "..GCC_OPTS, verbose)
	cmd("rm -rf "..SCRIPT_FILE, verbose)
end

dofile(arg[1])
