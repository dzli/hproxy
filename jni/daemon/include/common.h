#ifndef __FCT_COMMON_H__
#define __FCT_COMMON_H__

#include <unistd.h>

#define MAX_FILE_NAME 512 
#define DEFAULT_CONF_FILE "/etc/forticlient.conf"
#define MAX_VERSION_LEN    30

struct   fctl_conf_t{
	//path	
     char conf_file[MAX_FILE_NAME];
    char pid_file[MAX_FILE_NAME];
    char log_file[MAX_FILE_NAME];
    char iptables[MAX_FILE_NAME];
        
    //other
    int get_original_destination;
    int log_level;

    //http
    int enable_http;
    int http_ports[10];
    int http_proxy_listen_port;
    int http_min_bind_port;
    int http_max_bind_port; 

};

#define LOAD_STRING(conf , varname , varvalue , maxlen)\
do{\
	if  (! conf_get_string(conf , varname ,varvalue , maxlen)){\
		\
	}else{\
		ERRLOG("%s not found!" , varname);\
			return 2;\
	}\
}while(0)

#define LOAD_INT(conf , varname , varvalue)\
do{\
	int ivalue;\
		if  (! conf_get_int(conf , varname ,&ivalue)){\
			varvalue = ivalue;\
			\
		}\
		else{\
			ERRLOG("%s not found!" , varname);\
				return 2;\
		}\
}while(0)

#ifdef __cplusplus
extern "C" 
{
#endif

int  load_conf(struct   fctl_conf_t* conf, char* filename);
int copy_file(const char *infile, const char *outfile);
pid_t get_pidl(char* pidfile);
int write_pid(char* pidfile);
int get_pid(char* pidfile);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif
