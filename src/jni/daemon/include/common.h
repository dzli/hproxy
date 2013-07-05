#ifndef __FCT_COMMON_H__
#define __FCT_COMMON_H__

#include <unistd.h>

#define MAX_FILE_NAME 512 
#define DEFAULT_CONF_FILE "/etc/forticlient.conf"
#define MAX_VERSION_LEN    30

struct   fctl_conf_t{
	//path	
    char home_dir[MAX_FILE_NAME];
    char conf_file[MAX_FILE_NAME];
    char log_dir[MAX_FILE_NAME];
    char pid_file[MAX_FILE_NAME];
    char av_service[MAX_FILE_NAME];
    char avlog_service[MAX_FILE_NAME];
    char avrt_service[MAX_FILE_NAME];
    char avcrt_service[MAX_FILE_NAME];
    char log_file[MAX_FILE_NAME];
    char clamscan[MAX_FILE_NAME];
    char freshclam[MAX_FILE_NAME];
    char clamd_sock[MAX_FILE_NAME];
    char spool_dir[MAX_FILE_NAME];
    char iptables[MAX_FILE_NAME];
    char avlogd_sock[MAX_FILE_NAME];
    char avlog_file[MAX_FILE_NAME];
    char avlisten_sock[MAX_FILE_NAME];
    char av_quarantine_dir[MAX_FILE_NAME];
    char fctdb_file[MAX_FILE_NAME];
    char av_sig_dir[MAX_FILE_NAME];
        
    //other
    int get_original_destination;
    int log_level;
    char serial_number[50]; 
    char UID[20];
    unsigned int expiry_date;
    int enable_proxy;

    //http
    int enable_http;
    int http_ports[10];
    int http_proxy_listen_port;
    int http_min_bind_port;
    int http_max_bind_port; 

    //ftp
    int enable_ftp;
    int ftp_ports[10];
    int ftp_proxy_listen_port;
    int ftp_min_bind_port;  
    int ftp_max_bind_port; 

    //pop3
    int enable_pop3;
    int pop3_ports[10];
    int pop3_proxy_listen_port;
    int pop3_min_bind_port;  
    int pop3_max_bind_port; 

    //smtp
    int enable_smtp;
    int smtp_ports[10];
    int smtp_proxy_listen_port;
    int smtp_min_bind_port;  
    int smtp_max_bind_port; 
    
    //av
    int av_service_port;
    char av_engine_version[MAX_VERSION_LEN];
    char av_signature_version[MAX_VERSION_LEN];
    int av_enable_real_time ;
    int av_enable_pop3;
    int av_enable_smtp;
    int av_virus_fate;
    int av_real_time_virus_fate;
    int  av_cache_max_dir;
    int  av_cache_max_file;
    int av_max_open_scan_file; 
    // fortinet av engine
    char av_signature_path[MAX_FILE_NAME];
    char trust_file_cache[MAX_FILE_NAME]; 
    char av_update_server[50];
    char av_update_time[20];
    
    int av_real_time_valid;
    int av_rtscan_instance;
    int av_backendscan_instance;      

    int av_enable_grayware;               
    int av_enable_rt_grayware;            
    int av_enable_heuristic;               
    int av_enable_rt_heuristic;            
    int av_enable_compress_scan;           
    int av_compress_limit;                 
    int av_enable_rt_compress_scan;        
    int av_rt_compress_limit;              
    int av_rt_broadcast_console;
    int av_rt_alert_to_gui;
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
