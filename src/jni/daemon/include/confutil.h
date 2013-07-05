#ifndef __CONF_UTIL_H__
#define __CONF_UTIL_H__

#ifdef __cplusplus
extern "C" 
{
#endif

int conf_getstr(const char* name , char* value , int valuelen);
char* cfg_get_install_dir();
char* cfg_get_conf_file();
int cfg_get_av_sig_version_n(unsigned int *pMajor, unsigned int *pMinor);
int cfg_get_av_ext_sig_version_n(unsigned int *pMajor, unsigned int *pMinor);
int cfg_get_av_sig_version(char* sig_version);
int cfg_get_av_ext_sig_version(char* sig_version);

/*
 * caller must make sure the size of sig_version must be > 20
 */
int cfg_get_av_sig_version_full(char* sig_version);
/*
 * caller must make sure the size of sig_version must be > 20
 */
int cfg_get_av_ext_sig_version_full(char* sig_version);
int cfg_get_av_engine_version_n(unsigned int* major, unsigned int* minor, unsigned int* build, unsigned int* patch, char* new_engine);
int cfg_get_av_engine_version(char* buf, int size, char* new_engine);


#ifdef __cplusplus
}  /* end extern "C" */
#endif



#endif

