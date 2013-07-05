#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#ifdef __cplusplus
extern "C" 
{
#else
#define bool int
#endif

int mac_get_os_version(char *ver, int len);
bool SiInitialize(char* fct_home);
void SiCleanup();

//bool SiGenerateSysInfo(const char *pcszReq, std::string &info, const char *pcszUser);
int SiGenerateSysInfoC(const char *pcszReq, const char *pcszUser, char** outInfo);

#ifdef __cplusplus
}  /* end extern "C" */
#endif


#endif //__SYSINFO_H__


