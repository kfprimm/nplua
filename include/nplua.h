
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

#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void nplua_log(const char *format, ...);

int nplua_init();
char *nplua_name();
char *nplua_description();
char *nplua_mimedescription();
void nplua_close();

int nplua_new(const char *mime, uint16_t mode, int16_t argc, char *argn[], char *argv[]);

#endif
