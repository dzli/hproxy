#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include<string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h> 

#include <sys/un.h>
#include <fcntl.h>

#include "../include/log.h"

#ifdef __APPLE__
#include <asl.h>
#endif

void log_get_source_desc(int source, char* source_desc , int len);

static  int log_level  = FCTLOG_LEVEL_ERROR;
static char* sock_file = NULL;
static int log_source  = FCTLOG_SOURCE_PROXY;
static int  where = FCTLOG_TO_FILE;
static int log_file_fd = -1;

#ifdef __APPLE__

static aslclient asl = NULL;

void _init_asl(int source, int to_stderr)
{
	if (asl != NULL){
		asl_close(asl);;
		asl = NULL;
	}
    if (log_file_fd >= 0)
        close(log_file_fd);

	char source_desc[64];
	log_get_source_desc(source, source_desc, sizeof(source_desc));
	
	int opt = (to_stderr)?ASL_OPT_STDERR:0;
	asl = asl_open(FCT_LOG_IDENTITY, source_desc, opt);
    log_file_fd = open(sock_file,  O_WRONLY | O_CREAT | O_TRUNC, 0644);  
    if (log_file_fd >= 0)
        asl_add_log_file(asl, log_file_fd); 
}

int asl_log_level(int fct_log_level){
	switch (fct_log_level){
    	case FCTLOG_LEVEL_ERROR : return ASL_LEVEL_ERR;
    	case FCTLOG_LEVEL_ALERT : return ASL_LEVEL_WARNING;
    	case FCTLOG_LEVEL_INFO  : return ASL_LEVEL_NOTICE;
    	case FCTLOG_LEVEL_DEBUG : return ASL_LEVEL_DEBUG;
    	default:
    		return ASL_LEVEL_NOTICE;
    }
    return ASL_LEVEL_NOTICE;
};


#endif

void log_init(int level, int source , char* file){
    log_level = level;
    sock_file = file;
    log_source = source;
    
    #ifdef __APPLE__
    	_init_asl(source,0);
    #endif
}

void log_init_ex(int level, int source , char* file, int to){
    log_level = level;
    sock_file = file;
    log_source = source;
    where = to;
    
     #ifdef __APPLE__
    	_init_asl(source, (to ==  FCTLOG_TO_CONSOLE));
    #endif
}

static inline void __log(const char *szMsg, int level , va_list ap ){
    char level_str[32];
    log_get_level_desc(level, level_str, sizeof(level_str));
    
    time_t now;
    struct timeval tv;
 	time(&now);
    gettimeofday(&tv, 0);
  	char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %T", localtime(&now));
    
#ifdef __linux__
	if (where & FCTLOG_TO_CONSOLE){
		printf( "%s.%03d [%s] ",time_str, localtime(&now), level_str);
		vprintf(szMsg, ap);
		printf("\n");
	}
	if ((where & FCTLOG_TO_FILE) && sock_file != NULL){
        FILE* fp = fopen(sock_file, "a+");
        if (fp){
            fprintf(fp,  "%s.%03d [%s] ",time_str, localtime(&now), level_str);
            vfprintf(fp, szMsg, ap);
			fprintf(fp, "\n");
            fclose(fp);
        }
    }
#endif

#ifdef __APPLE__
   asl_log(asl, NULL, asl_log_level(level), szTmp);
#endif
}


void log_error(const char *szMsg, ... )
{       
        if (log_level < FCTLOG_LEVEL_ERROR)
            return;
            
		va_list marker;
		va_start( marker ,szMsg );  
		__log(szMsg , FCTLOG_LEVEL_ERROR, marker);
		va_end( marker );   
	
}

void log_warn(const char *szMsg, ... )
{
        if (log_level < FCTLOG_LEVEL_ALERT)
            return;
       
		va_list marker;
		va_start( marker ,szMsg );  
		__log(szMsg , FCTLOG_LEVEL_ALERT, marker);
		va_end( marker );   
	
}

void log_debug(const char *szMsg, ... )
{
       if (log_level < FCTLOG_LEVEL_DEBUG)
            return;
   		
		va_list marker;
		va_start( marker ,szMsg );  
		__log(szMsg , FCTLOG_LEVEL_DEBUG, marker);
		va_end( marker );   
		
	
}

void log_info(const char *szMsg, ... )
{
       if (log_level < FCTLOG_LEVEL_INFO)
            return;
       
		va_list marker;
		va_start( marker ,szMsg );  
		__log(szMsg , FCTLOG_LEVEL_INFO, marker);
		va_end( marker );   
}

void log_get_level_desc(int level, char* level_desc , int len){
    
    switch (level){
        case FCTLOG_LEVEL_ERROR :
            snprintf(level_desc, len , "%s", "ERROR");
            break;
        case FCTLOG_LEVEL_ALERT :
            snprintf(level_desc, len , "%s", "WARN");
            break;
        case FCTLOG_LEVEL_INFO :
            snprintf(level_desc, len , "%s", "INFO");
            break;
        case FCTLOG_LEVEL_DEBUG :
            snprintf(level_desc, len , "%s", "DEBUG");
            break;
                        
    }
}

int log_get_level_id(char* level_desc ){
    
    if (strcmp(level_desc , "ERROR") == 0){
        return FCTLOG_LEVEL_ERROR;
    }
    if (strcmp(level_desc , "WARN") == 0){
        return FCTLOG_LEVEL_ALERT;
    }
    
     if (strcmp(level_desc , "INFO") == 0){
        return FCTLOG_LEVEL_INFO;
    }
    if (strcmp(level_desc , "DEBUG") == 0){
        return FCTLOG_LEVEL_DEBUG;
    }
    
    return -1;
    
}

void log_get_source_desc(int source, char* source_desc , int len){
    
    switch (source){
        
        case FCTLOG_SOURCE_PROXY :
            snprintf(source_desc, len , "%s", "FortiProxy");
            break;
        
        case FCTLOG_SOURCE_AV :
            snprintf(source_desc, len , "%s", "AntiVirus");
            break;
        
        default:
            snprintf(source_desc, len , "%s", "Unknown");
            break;
   }
}

int log_get_source_id(char* source_desc ){
    
    if (strcmp(source_desc , "FortiProxy") == 0){
        return FCTLOG_SOURCE_PROXY;
    }
    if (strcmp(source_desc , "AntiVirus") == 0){
        return FCTLOG_SOURCE_AV;
    }
    
    return -1;
    
}
