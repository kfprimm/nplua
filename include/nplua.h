
#ifndef NPLUA_HEADER
#define NPLUA_HEADER

#ifdef WIN32

#include <windows.h>
typedef unsigned int bool;
#define false 0
#define true 1
#define OSDECL __declspec(dllexport)

#else

#define OSDECL

#endif

#ifndef HIBYTE
#define HIBYTE(x) ((((uint32_t)(x)) & 0xff00) >> 8)
#endif

#include <stdint.h>

#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef enum NPLUA_EVENT
{
	EVENT_INVALID,
	EVENT_MOUSEUP,
	EVENT_MOUSEDOWN,
	EVENT_COUNT
} NPLUA_EVENT;

void nplua_log(const char *format, ...);

int nplua_init();
char *nplua_name();
char *nplua_description();
char *nplua_mimedescription();
void nplua_close();

int nplua_new(const char *mime, uint16_t mode, int16_t argc, char *argn[], char *argv[]);
int nplua_destroy(int index);
#ifdef WIN32
void nplua_setwindow(int index, HWND hwnd, int width, int height);
int nplua_fromhwnd(HWND hwnd);
#endif
int nplua_windowed(int index);
void nplua_handleevent(int index, int event, int data, int x, int y);

void nplua_setpdata(int index, void *pdata);
void *nplua_getpdata(int index);

int nplua_hasmethod(int index, const char *name);

void nplua_pushmethod(int index, const char *name);
void nplua_pushnil();
void nplua_pushboolean(int boolean);
void nplua_pushnumber(double number);
void nplua_pushstring(const char *string);
void nplua_pushcdata(void *data);
int nplua_call(int argc);
int nplua_type();
const char *nplua_tostring();
int nplua_toboolean();
double nplua_tonumber();
void nplua_finish();


void nplua_register(lua_State *L);
int nplua_execute(lua_State *L);

#endif
