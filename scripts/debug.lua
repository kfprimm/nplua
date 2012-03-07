print("\nMIME string: ".._npmime)
for key,value in pairs(_npdecls) do
	print(key.." = "..type(value))
	print(type(value.__index))
end
