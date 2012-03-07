
#include <nplua.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


static NPNetscapeFuncs *npnfuncs;

//lazy
void _npapi_set_string(NPVariant *v, char *txt, int length)
{
	char *t = (char *)npnfuncs->memalloc(length);
	strcpy(t, txt);
	STRINGN_TO_NPVARIANT(t, length, *v);
}

char *_npapi_get_string(NPVariant *v)
{
	return (char*)NPVARIANT_TO_STRING(*v).UTF8Characters;
}

void _npapi_get_url_notify(NPP instance, const  char* url, const  char* target, void* notifyData)
{

	npnfuncs->geturlnotify(instance, url, target, npnfuncs);
}

void _npapi_post_url_notify(NPP instance, const char* url, const char* target, long len, const char* buf, NPBool file, void* notifyData)
{
	npnfuncs->posturl(instance, url, target, (uint32_t)len, buf, file);
}

char *_npapi_get_page_url(NPP instance) {
	NPObject *window = 0;
	npnfuncs->getvalue( instance, NPNVWindowNPObject, &window );
	NPIdentifier identifier = npnfuncs->getstringidentifier( "location" );
	NPVariant variant;
	npnfuncs->getproperty( instance, window , identifier, &variant);
	NPObject *location = variant.value.objectValue;
	identifier = npnfuncs->getstringidentifier( "href" );
	npnfuncs->getproperty( instance, location, identifier, &variant);
	return (char*)NPVARIANT_TO_STRING(variant).UTF8Characters;
}

typedef struct NPO
{
	NPObject np;
	void *pdata;
} NPO;

typedef struct PDATA
{
	NPO *npo;
	int index;
} PDATA;

NPObject *NPO_Allocate(NPP instance, NPClass *aClass)
{
	nplua_log("NPO_Allocate!");
	return (NPObject*)malloc(sizeof(NPO));
}

static bool NPO_HasMethod(NPObject* obj, NPIdentifier methodName) {
	nplua_log("NPO_HasMethod!");

	PDATA *pd = (PDATA*)((NPO*)obj)->pdata;


	int result = 0;
	char *name = npnfuncs->utf8fromidentifier(methodName);
	result = nplua_hasmethod(pd->index, name);
	npnfuncs->memfree(name);
	return result;
}

static void NPO_PushArgs(const NPVariant *args, uint32_t argCount)
{
	for (int i = 0;i < argCount;i++)
	{
		switch (args[i].type)
		{
		case NPVariantType_Null:case NPVariantType_Void: nplua_pushnil();break;
		case NPVariantType_Bool:   nplua_pushboolean(NPVARIANT_TO_BOOLEAN(args[i]));break;
		case NPVariantType_Int32:  nplua_pushnumber(NPVARIANT_TO_INT32(args[i]));break;
		case NPVariantType_Double: nplua_pushnumber(NPVARIANT_TO_DOUBLE(args[i]));break;
		case NPVariantType_String: nplua_pushstring(NPVARIANT_TO_STRING(args[i]).UTF8Characters);break;
		case NPVariantType_Object: nplua_pushcdata(NPVARIANT_TO_OBJECT(args[i]));break;
		}
	}

}

static void NPO_HandleResult(NPVariant *result)
{
	switch (nplua_type())
	{
	case LUA_TSTRING:
	{
		int length = strlen(nplua_tostring());
		char *t = (char *)npnfuncs->memalloc(length);
		strcpy(t, nplua_tostring());
		STRINGN_TO_NPVARIANT(t, length, *result);
		break;
	}
	default:
		BOOLEAN_TO_NPVARIANT(1, *result);
		break;
	}
}

static bool NPO_InvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	PDATA *pd = (PDATA*)((NPO*)obj)->pdata;
	int valid = 0;
	if (nplua_hasmethod(pd->index, "Invoke"))
	{
		nplua_pushmethod(pd->index, "Invoke");
		NPO_PushArgs(args, argCount);

		valid = nplua_call(argCount);

		if (valid)
			NPO_HandleResult(result);
		else
			nplua_log("ERROR: %s", nplua_tostring());

		nplua_finish();
	}
	return valid;
}

static bool NPO_Invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	PDATA *pd = (PDATA*)((NPO*)obj)->pdata;

	char *name = npnfuncs->utf8fromidentifier(methodName);
	nplua_pushmethod(pd->index, name);
	npnfuncs->memfree(name);

	NPO_PushArgs(args, argCount);
	int valid = nplua_call(argCount);

	if (valid)
		NPO_HandleResult(result);
	else
		nplua_log("ERROR: %s", nplua_tostring());

	nplua_finish();

	return valid;
}

static bool NPO_HasProperty(NPObject *obj, NPIdentifier propertyName) {
	nplua_log("NPO_HasProperty!");

	return false;
}

static bool NPO_GetProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result) {
	nplua_log("NPO_GetProperty!");

	return false;
}

NPClass npoObject = {
	NP_CLASS_STRUCT_VERSION,
	NPO_Allocate,
	NULL,
	NULL,
	NPO_HasMethod,
	NPO_Invoke,
	NPO_InvokeDefault,
	NPO_HasProperty,
	NPO_GetProperty,
	NULL,
	NULL,
};

static NPError NPO_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved)
{
	int index = nplua_new((const char *)pluginType, mode, argc, argn, argv);
	if (index == 0)
		return NPERR_GENERIC_ERROR;

	PDATA *pd = (PDATA*)malloc(sizeof(PDATA));
	pd->npo = 0;
	pd->index = index;

	instance->pdata = pd;

	return NPERR_NO_ERROR;
}

static NPError NPO_Destroy(NPP instance, NPSavedData **save)
{
	nplua_log("NPO_Destroy!");

	PDATA *pd = (PDATA*)instance->pdata;
	npnfuncs->releaseobject((NPObject*)pd->npo);

	free(instance->pdata);
	return NPERR_NO_ERROR;
}

static NPError NPO_GetValue(NPP instance, NPPVariable variable, void *value)
{
	nplua_log("NPO_GetValue = %i!", variable);

	PDATA *pd = (PDATA*)instance->pdata;
	NPObject* npobject = (NPObject*)pd->npo;

	switch(variable) {
	case NPPVpluginWindowBool:
		*((int *)value) = nplua_windowed(pd->index);
		break;
	case NPPVpluginNameString:
		*((char **)value) = nplua_name();
		break;
	case NPPVpluginDescriptionString:
		*((char **)value) = nplua_description();
		break;
	case NPPVpluginScriptableNPObject:
		if (npobject == NULL)
		{
			npobject = npnfuncs->createobject(instance, &npoObject);
			pd->npo = (NPO*)npobject;
			pd->npo->pdata = (void*)pd;
		}
		npnfuncs->retainobject(npobject);
		*(NPObject **)value = npobject;
		break;
	case NPPVpluginNeedsXEmbed:
		*((int *)value) = 1;
		break;
	default:
		return NPERR_GENERIC_ERROR;
	}
	return NPERR_NO_ERROR;
}

static NPError NPO_HandleEvent(NPP instance, void *ev)
{
	nplua_log("NPO_HandleEvent!");

	//NPEvent *event = (NPEvent*)ev;
#ifdef WIN32
#endif
	return NPERR_NO_ERROR;
}

static NPError NPO_SetWindow(NPP instance, NPWindow* window)
{
#ifdef WIN32
	HWND hwnd;
	npnfuncs->getvalue(instance, NPNVnetscapeWindow, (void*) &hwnd);
#endif
	nplua_log("NPO_SetWindow!");
	return NPERR_NO_ERROR;
}

static NPError NPO_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool  seekable, uint16_t* stype)
{
	nplua_log("NPO_NewStream!");
	*stype = NP_ASFILEONLY;
	return NPERR_NO_ERROR;
}

static NPError NPO_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{
	nplua_log("NPO_DestroyStream!");
	return NPERR_NO_ERROR;
}

static void NPO_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
	nplua_log("NPO_URLNotify!");
}

static void NPO_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
	nplua_log("NPO_StreamAsFile!");
}

static int32_t NPO_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buf)
{
	nplua_log("NPO_Write!");
	return 0;
}

static int32_t NPO_WriteReady(NPP instance, NPStream* stream)
{
	nplua_log("NPO_WriteReady!");
	return 0;
}

OSDECL NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *nppfuncs) {
	if(nppfuncs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if(nppfuncs->size < sizeof(NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	nppfuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	nppfuncs->newp          = NPO_New;
	nppfuncs->destroy       = NPO_Destroy;
	nppfuncs->getvalue      = NPO_GetValue;
	nppfuncs->event         = NPO_HandleEvent;
	nppfuncs->setwindow     = NPO_SetWindow;
	nppfuncs->newstream     = NPO_NewStream;
	nppfuncs->destroystream = NPO_DestroyStream;
	nppfuncs->urlnotify     = NPO_URLNotify;
	nppfuncs->asfile        = NPO_StreamAsFile;
	nppfuncs->write         = NPO_Write;
	nppfuncs->writeready    = NPO_WriteReady;

	return NPERR_NO_ERROR;
}

OSDECL NPError OSCALL NP_Initialize(NPNetscapeFuncs *npnf
#ifdef WIN32
                             )
#else
                             ,NPPluginFuncs *nppfuncs)
#endif
{
	if(npnf == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if(HIBYTE(npnf->version) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	npnfuncs = npnf;

	nplua_init();

#ifndef WIN32
	NP_GetEntryPoints(nppfuncs);
#endif

	return NPERR_NO_ERROR;
}

OSDECL NPError OSCALL NP_Shutdown() {
	nplua_close();
	return NPERR_NO_ERROR;
}

#if NP_VERSION_MINOR >= 27
const
#endif
char *NP_GetMIMEDescription(void)
{
	return (char*)nplua_mimedescription();
}

NPError OSCALL NP_GetValue(void *npp, NPPVariable variable, void *value)
{
	return NPO_GetValue((NPP)npp, variable, value);
}

