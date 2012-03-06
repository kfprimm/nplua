
#include <nplua.h>
#include <string.h>
#include <setjmp.h>

char _plugin_name[100];
char _plugin_desc[100];
char _plugin_mime[256];

void nplua_log(const char *format, ...)
{
  char msg[255];
  va_list list;
  va_start(list,format);
  vsprintf(msg,format,list);
  va_end(list);
  FILE *file = fopen("/home/kfprimm/nplua/debug.log", "a");
  fprintf(file, "%s\n", msg);
  fflush(file);
  fclose(file);
}

static int nplua_print(lua_State *L) {
	if (lua_type(L, 1) == LUA_TSTRING)
		nplua_log(lua_tostring(L, 1));
	return 0;
}

static int nplua_npobject(lua_State *L) {
	lua_pushvalue(L, 1);
  return 1;
}

static int nplua_npplugin(lua_State *L) {
	lua_pushvalue(L, 1);
	lua_setglobal(L, "_npplugin");
	
	lua_pushvalue(L, 1);
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		lua_pushvalue(L, -2);
		const char *key = lua_tostring(L, -1);
		if (!strcmp(key, "Name"))
			strcpy(_plugin_name, lua_tostring(L, -2));
		else if (!strcmp(key, "Description"))
			strcpy(_plugin_desc, lua_tostring(L, -2));
		else if (!strcmp(key, "Objects"))
		{
			lua_newtable(L);
			int npobjects = lua_gettop(L);
			lua_pushstring(L, "");
			int mimestring = lua_gettop(L), index = 0;
			
			lua_pushvalue(L, -4);
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				lua_pushvalue(L, -2);

				lua_pushnumber(L, 3);lua_gettable(L, -3);
				lua_pushnumber(L, 2);lua_gettable(L, -4);
				lua_pushnumber(L, 1);lua_gettable(L, -5);
			
				lua_pushvalue(L, -2);
				lua_pushvalue(L, -2);
				lua_settable(L, npobjects);				
				
				lua_pushvalue(L, mimestring);
				if (index != 0)
				{
					lua_pushstring(L, ";");
					lua_concat(L, 2);
				}
				lua_pushvalue(L, -3);
				lua_pushstring(L, "::");
				lua_pushvalue(L, -6);
				lua_concat(L, 4);
				lua_replace(L, mimestring);
				
				lua_pop(L, 3);

				index += 1;
				lua_pop(L, 2);
			}
			lua_pop(L, 1);

			strcpy(_plugin_mime, lua_tostring(L, -1));
			lua_setglobal(L, "_npmime");
			
			lua_setglobal(L, "_npdecls");
		}
		lua_pop(L, 2);
	}
	lua_pop(L, 1);

  return 0;
}

lua_State *L = 0;
static jmp_buf jmp;
//				nplua_log("-- %s - %s", lua_typename(L, lua_type(L,-1)), lua_typename(L, lua_type(L,-2)));

static int nplua_panic(lua_State *L) {
	nplua_log("ERROR: %s", lua_tostring(L, 1));
	longjmp(jmp,1);
}

int nplua_init()
{
	if (L != 0)
		return true;
		
	L = lua_open();
	luaL_openlibs(L);
	lua_atpanic(L, nplua_panic);
	
	lua_pushcfunction(L, nplua_npobject);
	lua_setglobal(L, "NPObject");
	
  lua_pushcfunction(L, nplua_npplugin);
  lua_setglobal(L, "NPPlugin");
  
	lua_pushcfunction(L, nplua_print);
	lua_setglobal(L, "print");

	if (!setjmp(jmp))
	{	
		if (luaL_dofile(L, "/home/kfprimm/nplua/test/nphelloworld.lua") != 0)
		{
			nplua_log("ERROR: %s", lua_tostring(L, -1));
			return false;
		}

		if (luaL_dofile(L, "/home/kfprimm/nplua/bin/debug.lua") != 0)
		{
			nplua_log("ERROR: %s", lua_tostring(L, -1));
			return false;
		}
  }
	
  return true;
}

char *nplua_name() { nplua_init(); return _plugin_name; }
char *nplua_description() { nplua_init(); return _plugin_desc; }
char *nplua_mimedescription() { nplua_init(); return _plugin_mime; }

int nplua_new(const char *mime, uint16_t mode, int16_t argc, char *argn[], char *argv[])
{
	return 0;
	int index = 0;
	lua_getglobal(L, "_npdecls");
	lua_pushstring(L, mime);
	lua_gettable(L, -2);
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		
		index = luaL_getn(L, -2);
	}
	lua_pop(L, 2);
	
}

void nplua_close()
{
	lua_close(L);
}
