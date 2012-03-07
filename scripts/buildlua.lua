
--", "Lua", "build

LUA_O = { "lapi", "lcode", "ldebug", "ldo", "ldump", "lfunc", "lgc", "llex", "lmem", "lobject", "lopcodes", "lparser", "lstate", "lstring", "ltable", "ltm", "lundump", "lvm", "lzio", "lauxlib", "lbaselib", "ldblib", "liolib", "lmathlib", "loslib", "ltablib", "lstrlib", "loadlib", "linit"}
AR_CMD = "ar rcs lib/liblua51.a"

for i,src in ipairs(LUA_O) do
	print("Building "..src..".c...")
	os.execute("gcc -c -o src/"..src..".o src/lua/"..src..".c")
	AR_CMD = AR_CMD.." src/"..src..".o"
end

print("Linking lib/liblua51.a...")
os.execute(AR_CMD)


