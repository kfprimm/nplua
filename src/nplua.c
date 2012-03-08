
#include <nplua.h>
#include <string.h>
#include <setjmp.h>

//#define NPLUA_PATH "/home/kfprimm/nplua"
#define NPLUA_PATH "C:\\Users\\Kevin\\Projects\\nplua"


char _plugin_name[100];
char _plugin_desc[100];
char _plugin_mime[256];

void nplua_log(const char *format, ...)
{
#ifdef DEBUG
	char msg[255];
	va_list list;
	va_start(list,format);
	vsprintf(msg,format,list);
	va_end(list);
	FILE *file = fopen(NPLUA_PATH "/debug.log", "a");
	fprintf(file, "%s\n", msg);
	fflush(file);
	fclose(file);
#endif
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

				lua_pushstring(L, "__index");
				lua_pushvalue(L, -2);
				lua_settable(L, -3);

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

	nplua_register(L);

	if (!setjmp(jmp))
	{
		if (nplua_execute(L) != 0)
		{
			nplua_log("ERROR: %s", lua_tostring(L, -1));
			return false;
		}

		lua_newtable(L);
		lua_setglobal(L, "_npobjects");
#ifdef DEBUG
		if (luaL_dofile(L, NPLUA_PATH "/scripts/debug.lua") != 0)
		{
			nplua_log("ERROR: %s", lua_tostring(L, -1));
			return false;
		}
#endif
  }

  return true;
}

char *nplua_name() { nplua_init(); return _plugin_name; }
char *nplua_description() { nplua_init(); return _plugin_desc; }
char *nplua_mimedescription() { nplua_init(); return _plugin_mime; }

int nplua_new(const char *mime, uint16_t mode, int16_t argc, char *argn[], char *argv[])
{
	nplua_log("Looking for %s...", mime);
	int index = 0;
	lua_getglobal(L, "_npdecls");
	lua_pushstring(L, mime);
	lua_gettable(L, -2);
	nplua_log("type = %s", lua_typename(L, lua_type(L, -1)));
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		lua_newtable(L);
		lua_pushvalue(L, -2);
		lua_setmetatable(L, -2);

		lua_getglobal(L, "_npobjects");
		index = luaL_getn(L, -1) + 1;
		lua_pushnumber(L, index);
		lua_pushvalue(L, -3);
		lua_settable(L, -3);

		lua_newtable(L);
		lua_pushnumber(L, 0);
		lua_setfield(L, -2, "HWND");
		lua_pushnumber(L, 0);
		lua_setfield(L, -2, "Width");
		lua_pushnumber(L, 0);
		lua_setfield(L, -2, "Height");
		lua_setfield(L, -3, "Window");

		lua_getfield(L, -2, "New");
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			lua_pushvalue(L, -3);
			lua_newtable(L);
			for (int i = 0;i < argc;i++)
			{
				lua_pushstring(L, argn[i]);
				lua_pushstring(L, argv[i]);
				lua_settable(L, -3);
			}
			lua_pushnumber(L, mode);
			if (lua_pcall(L, 3, 0, 0) != 0)
			{
				nplua_log("ERROR: %s", lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}

		lua_pop(L, 3);
	}
	lua_pop(L, 2);
	nplua_log("new! index = %i", index);
	return index;
}

int nplua_destroy(int index)
{
	if (nplua_hasmethod(index, "Destroy"))
	{
		nplua_pushmethod(index, "Destroy");
		if (!nplua_call(0))
		{
			nplua_log("ERROR: %s", nplua_tostring());
			lua_pop(L, 1);
			return 0;
		}
	}
	return 1;
}

int nplua_windowed(int index)
{
	int result = 0;
	lua_getglobal(L, "_npobjects");
	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	lua_getfield(L, -1, "Options");
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		lua_getfield(L, -1, "Windowed");
		lua_pushboolean(L, 1);
		index = lua_equal(L, -1, -2);
		lua_pop(L, 2);
	}

	lua_pop(L, 2);
	return result;
}

void nplua_setwindow(int index, HWND hwnd, int width, int height)
{
	lua_getglobal(L, "_npobjects");
	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	lua_pushnumber(L, hwnd);
	lua_setfield(L, -2, "Window");

	lua_pushnumber(L, width);
	lua_setfield(L, -2, "Width");

	lua_pushnumber(L, height);
	lua_setfield(L, -2, "Height");

	lua_pop(L, 1);
}

int nplua_hasmethod(int index, const char *name)
{
	int result = 0;
	lua_getglobal(L, "_npobjects");
	lua_pushnumber(L, index);
	lua_gettable(L, -2);

	lua_getfield(L, -1, name);
	if (lua_type(L, -1) == LUA_TFUNCTION)
		result = 1;

	nplua_log("HAS METHOD? %s = %i", name, result);

	lua_pop(L, 2);
	return result;
}

void nplua_pushmethod(int index, const char *name)
{
	lua_getglobal(L, "_npobjects");
	lua_pushnumber(L, index);
	lua_gettable(L, -2);
	lua_getfield(L, -1, name);
	lua_pushvalue(L, -2);
}
void nplua_pushnil() { lua_pushnil(L); }
void nplua_pushboolean(int boolean) { lua_pushboolean(L, boolean); }
void nplua_pushnumber(double number) { lua_pushnumber(L, number); }
void nplua_pushstring(const char *string) { lua_pushstring(L, string); }
void nplua_pushcdata(void *data) { lua_pushlightuserdata(L, data); }
int nplua_call(int argc)
{
	if (lua_pcall(L, argc + 1,1,0) != 0)
		return 0;
	return 1;
}
int nplua_type() { return lua_type(L, -1); }
const char *nplua_tostring() { return lua_tostring(L, -1); }
int nplua_toboolean() { return nplua_toboolean(L, -1); }
double nplua_tonumber() { return nplua_tonumber(L, -1); }

void nplua_finish() { lua_pop(L, 2); }

void nplua_close()
{
	lua_close(L);
}
