#ifndef FCTL_LOG_H
#define FCTL_LOG_H



#define FCT_DEFAULT_LOG_FILE "/var/log/forticlient.log"
/*
	Log level defination
*/
enum fct_log_level{
    FCTLOG_LEVEL_ERROR  = 1,
    FCTLOG_LEVEL_ALERT = 2,
    FCTLOG_LEVEL_INFO  = 3,
    FCTLOG_LEVEL_DEBUG = 4
};

/*
	Log source
*/

enum fct_log_source{
    FCTLOG_SOURCE_PROXY =0,
    FCTLOG_SOURCE_HTTP ,
    FCTLOG_SOURCE_FTP,
    FCTLOG_SOURCE_POP3,
    FCTLOG_SOURCE_SMTP,
    FCTLOG_SOURCE_AV,
    FCTLOG_SOURCE_AV_REALTIME,
    FCTLOG_SOURCE_UPDATE
};

/*
	Where does log will to
*/
enum fct_log_dest{
    FCTLOG_TO_CONSOLE,
    FCTLOG_TO_SOCK
};


#ifdef __cplusplus
extern "C" 
{
#endif

/*
	log_init or log_init_ex must be called before output log.
	For Mac , sock parameter is not useful , just give it a NULL.
*/
void log_init(int level, int source , char* file);
void log_init_ex(int level, int source , char* file, int to);

#define DBGLOG(fmt, args ...) log_debug(fmt, ##args);
#define INFOLOG(fmt, args ...) log_info(fmt, ##args);
#define ERRLOG(fmt, args ...)  log_error(fmt, ##args);
#define WARNLOG(fmt, args ...)  log_warn(fmt, ##args);

void log_error(const char *szMsg, ... );
void log_warn(const char *szMsg, ... );
void log_debug(const char *szMsg, ... );
void log_info(const char *szMsg, ... );



struct fct_log_t{
    int type;
    int source;
    int level;
    unsigned long time;
    char content[512];
};



enum fct_log_type{
    FCTLOG_AV = 0,
    FCTLOG_GENERAL = 1,
    FCTLOG_UPDATE,
	FCTLOG_CMD,
    FCTLOG_CONF_REFRESH,
};




#define FCT_LOG_IDENTITY "com.fortinet.forticlient"

void log_get_level_desc(int level, char* level_desc , int len);
void log_get_source_desc(int source, char* source_desc , int len);
int log_get_source_id(char* source_desc );
int log_get_level_id(char* level_desc );

#ifdef __cplusplus
}  /* end extern "C" */
#endif



#endif
