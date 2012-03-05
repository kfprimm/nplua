
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
  void *ptr;
} NPO;

NPObject *NPO_Allocate(NPP npp, NPClass *aClass)
{
  NPO*object = (NPO*)malloc(sizeof(NPO));
  object->ptr = NULL;
  return &object->np;
}

static bool NPO_HasMethod(NPObject* obj, NPIdentifier methodName) {
	NPO *bmx = (NPO*)obj;
	char *name = npnfuncs->utf8fromidentifier(methodName);
	
	npnfuncs->memfree(name);
	return false;
}

static bool NPO_InvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	NPO *bmx = (NPO*)obj;
	return false;
}

static bool NPO_Invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	NPO *bmx = (NPO*)obj;
	char *name = npnfuncs->utf8fromidentifier(methodName);
	
	npnfuncs->memfree(name);
	return false;
}

static bool NPO_HasProperty(NPObject *obj, NPIdentifier propertyName) {
	return false;
}

static bool NPO_GetProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result) {
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
	nplua_log("NEW!");
	if (instance->pdata == NULL)
		return NPERR_GENERIC_ERROR;
	
	//npnfuncs->setvalue(instance, NPPVpluginWindowBool, (void*)false);

	return NPERR_NO_ERROR;
}

static NPError NPO_Destroy(NPP instance, NPSavedData **save)
{
	return NPERR_NO_ERROR;
}

static NPError NPO_GetValue(NPP instance, NPPVariable variable, void *value)
{
	NPO *bmx = NULL;
	int so = 0;

	switch(variable) {
	default:
		return NPERR_GENERIC_ERROR;
	case NPPVpluginNameString:
		*((char **)value) = nplua_name();
		break;
	case NPPVpluginDescriptionString:
		*((char **)value) = nplua_description();
		break;
	case NPPVpluginScriptableNPObject:		
		if (bmx == NULL && so)
		{
			bmx = (NPO*)npnfuncs->createobject(instance, &npoObject);
			npnfuncs->retainobject((NPObject*)bmx);
			bmx->ptr = instance->pdata;
		}
		*(NPO **)value = bmx;
		break;
	case NPPVpluginNeedsXEmbed:
		*((int *)value) = 1;
		break;
	}
	return NPERR_NO_ERROR;
}

static NPError OSCALL NPO_HandleEvent(NPP instance, void *ev)
{
	NPEvent *event = (NPEvent*)ev;
#ifdef WIN32
#endif
	return NPERR_NO_ERROR;
}

static NPError OSCALL NPO_SetWindow(NPP instance, NPWindow* window)
{
#ifdef WIN32
	HWND hwnd;
	npnfuncs->getvalue(instance, NPNVnetscapeWindow, (void*) &hwnd);
#endif
	return NPERR_NO_ERROR;
}

static NPError OSCALL NPO_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool  seekable, uint16_t* stype)
{	
	*stype = NP_ASFILEONLY;
	return NPERR_NO_ERROR;
}

static NPError OSCALL NPO_DestroyStream(NPP instance, NPStream* stream, NPReason reason)
{		
	return NPERR_NO_ERROR;
}

static void OSCALL NPO_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
}

static void OSCALL NPO_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
}

static int32_t OSCALL NPO_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buf)
{
	return 0;
}

static int32_t OSCALL NPO_WriteReady(NPP instance, NPStream* stream)
{
	return 0;
}

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *nppfuncs) {
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

NPError OSCALL NP_Shutdown() {
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

