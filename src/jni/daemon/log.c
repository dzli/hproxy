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
static int  where = FCTLOG_TO_SOCK;
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


static int send_log(struct fct_log_t* log){
    int     ret = 0, sockfd = -1;
    struct sockaddr_un cliaddr, servaddr;

    if (sock_file == NULL)
        return -4;
        
    sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0){
        return -3;
    }

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, sock_file);
    
    int servlen = sizeof(struct sockaddr_un);
    for(;;){
        int n = sendto(sockfd, log, sizeof(struct fct_log_t), 0, 
            (struct sockaddr*)&servaddr, servlen);
        if (n < 0){
            if (errno == EINTR) continue;
            ret = -1;
            break;    
        }
        if (n < sizeof(struct fct_log_t)){
            ret = -2;
        }
        break;
    }
    #define SAFE_CLOSE(x)	if (x > 0)	{while (close(x) == -1 && errno == EINTR);}
        
    SAFE_CLOSE(sockfd);
    
    return ret;
}

/*
static inline void __log(const char *szMsg, va_list ap ){
    char buffer1[128];
    
    time_t now;
    struct timeval tv;

	va_list marker;
	char szTmp[4096];

    time(&now);
    gettimeofday(&tv, 0);

    strftime(buffer1, sizeof(buffer1), "%Y/%m/%d %T", localtime(&now));
    snprintf(szTmp, sizeof(szTmp), "%s.%03d ", buffer1, (int)tv.tv_usec / 1000);
 
    int len = strlen(szTmp);
	vsnprintf(szTmp + len,sizeof(szTmp)- len, szMsg, ap);
	           

    fprintf(stderr, "%s\n", szTmp);
}*/


static inline void __log(const char *szMsg, int level , va_list ap ){
    char buffer1[128];
    
    time_t now;
    struct timeval tv;

	va_list marker;
	char szTmp[4096];

	vsnprintf(szTmp ,sizeof(szTmp), szMsg, ap);
	           
    struct fct_log_t log;
    log.type = FCTLOG_GENERAL;
    log.source = log_source;
    log.level = level;
    log.time = time(0);
    snprintf(log.content, sizeof(log.content) , "%s", szTmp);

#ifdef __linux__
 //   if (where == FCTLOG_TO_SOCK)
 //       send_log(&log);
 //   else{
        printf("%s\n", szTmp);
        FILE* fp = fopen("/tmp/proxy.log", "a+");
        if (fp){
            fprintf(fp, "%s\n", szTmp);
            fclose(fp);
        }
 //   }
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
