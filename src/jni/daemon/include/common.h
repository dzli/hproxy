#ifndef __FCT_COMMON_H__
#define __FCT_COMMON_H__

#include <unistd.h>

#ifdef __cplusplus
extern "C" 
{
#endif

int copy_file(const char *infile, const char *outfile);
pid_t get_pidl(char* pidfile);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif
