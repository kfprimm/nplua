
-- nphelloworld - nplua basic example

HelloObject = NPObject({
	Init = function(self, args, mode)
		for key,value in pairs(args) do
			print("args['"..key.."'] = '"..value.."'")
		end
	end,
	Run = function(self)
		return "Hello World!"
	end
})

EchoObject = NPObject({
	Run = function(self, string)
		return string
	end
})

NPPlugin({
	Name        = "nplua Example Plugin",
	Author      = "John Doe",
	Copyright   = "Copyright 2012 John Doe",
	Description = "A basic example plugin written in Lua.",
  PluginName  = "nphelloworld",
	Objects = {
		{ HelloObject, "application/x-nplua-hello", "An object that returns 'Hello World' from it's 'Run'." },
		{ EchoObject,  "application/x-nplua-echo",  "An object that echos anything passed to its 'Run' method." }
	}
})

