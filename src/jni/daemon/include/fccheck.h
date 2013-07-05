#ifndef __FCCHECK_H__
#define __FCCHECK_H__

//#include <string>
#include "../../include/compat_win32.h"

#ifdef __cplusplus
extern "C" 
{
#endif

void FcckInit();
void FcckCleanup();

void FcckUpdateCookie();

int FcckGetCookie(const char *seed /* = NULL */, char* encoded, int max_cookie_size);
unsigned long FcckGetFeatureSet(char *lpszBuffer, DWORD dwSize);

#ifdef __cplusplus
}  /* end extern "C" */
#endif


#endif

