#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "common.h"
#include "conf.h"
#include "log.h"


static inline void split_ports(int *ports , char *buf){
	char *port = strtok(buf , ",");
	int ports_size = 0;

	while(port){
		ports[ports_size] = atoi(port);
		ports_size++;
		port = strtok(NULL, ",");
	}
	ports[ports_size] = 0;
}

int  load_conf(struct   fctl_conf_t* conf, char* filename){
    if (conf == NULL){
        ERRLOG("conf is NULL!");
        return -1;
    } 
    
    char ports[128];
    
    struct ck_conf *pconf = conf_open(filename);
	if (!pconf){
		ERRLOG("can not open configuration file  %s\n", filename);
		return -1;
	}     
	
	LOAD_STRING(pconf , "conf_file" , conf->conf_file , MAX_FILE_NAME);
	LOAD_STRING(pconf , "log_file" ,   conf->log_file, MAX_FILE_NAME);
	LOAD_STRING(pconf , "pid_file" ,   conf->pid_file , MAX_FILE_NAME);
    LOAD_STRING(pconf , "iptables" ,   conf->iptables , MAX_FILE_NAME);
    
    LOAD_INT(pconf , "get_original_destination", conf->get_original_destination);
    LOAD_INT(pconf , "log_level", conf->log_level);
    
	LOAD_INT(pconf , "enable_http" , conf->enable_http);
	LOAD_INT(pconf , "http_proxy_listen_port" , conf->http_proxy_listen_port);
	LOAD_INT(pconf , "http_min_bind_port" , conf->http_min_bind_port);
	LOAD_INT(pconf , "http_max_bind_port" , conf->http_max_bind_port);
    LOAD_STRING(pconf , "http_port" , ports , sizeof(ports));
    split_ports(conf->http_ports, ports);
    
	
    conf_close(pconf);
	
	return 0;
}

 char* strncasestr(char* buf , int buflen , char*str){
    int i = 0;
	int str_len = strlen(str);
	int maxi = buflen - str_len ;
	
	while (i <= maxi && strncasecmp(buf+i, str, str_len) ) i++;
	
	if (i > maxi) return NULL;
	
	return buf+i;
}

void format_time(time_t time_int , char*time_str , int len ){
    strftime(time_str, len, "%m/%d/%Y %T", localtime(&time_int));
}

void format_date(time_t time_int , char*time_str , int len ){
    strftime(time_str, len, "%m/%d/%Y", localtime(&time_int));
}


static int lockfile(int fd){
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}

static pid_t file_is_locked(int fd)
{
    struct flock lock;
    lock.l_type = F_WRLCK;    // F_RDLCK or F_WRLCK 
    lock.l_start = 0;  // byte offset, relative to l_whence 
    lock.l_whence = SEEK_SET; // SEEK_SET, SEEK_CUR, SEEK_END 
    lock.l_len = 0;       // #bytes (0 means to EOF) 

    if (fcntl(fd, F_GETLK, &lock) < 0)
       return (0);

    if (lock.l_type == F_UNLCK)
        return (0);      // false, region isn't locked by another proc 
    return(lock.l_pid); // true, return pid of lock owner 
}


int write_pid(char* pidfile){
    int     fd;
    char    buf[16];

    fd = open(pidfile, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        ERRLOG("can't open %s: %s", pidfile, strerror(errno));
        return -1;
    }
    if (lockfile(fd) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        ERRLOG("can't lock %s: %s", pidfile, strerror(errno));
        return -2;
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf)+1);
    
    return(0);

}

int get_pid(char* pidfile){
    FILE *stream = NULL;
    int pid = 0;
    
    if (pidfile == NULL){
        return -1;
    }
    stream = fopen(pidfile , "r");
    if (stream == NULL){
        ERRLOG("can not read pid file %s" , pidfile);
        return pid;
    }
    
    fscanf(stream, "%d", &pid);
    fclose(stream);
    
    return pid;

}

pid_t get_pidl(char* pidfile){
    FILE *stream = NULL;
    pid_t pid = 0;
    
    if (pidfile == NULL){
        return -1;
    }
    stream = fopen(pidfile , "r");
    if (stream == NULL){
        ERRLOG("can not read pid file %s" , pidfile);
        return pid;
    }
    
    return file_is_locked(fileno(stream));

    //fscanf(stream, "%u", &pid);
    //fclose(stream);
    
    return pid;

}

int copy_file(const char *infile, const char *outfile) 
{
    FILE *infd, *outfd;
    char buf[4096];
    int len, err = 0;

    infd = fopen(infile, "r");
    if (infd == NULL) return -1;
    //	if (_taccess(outfile, 0) == 0)
    //		_tchmod(outfile, _S_IREAD | _S_IWRITE);

    outfd = fopen(outfile, "w"); 
    if (outfd == NULL) {
        fclose(infd);
        return -2;
    }

    while (!feof(infd)) {
        len = fread(buf, sizeof ( char), 4096, infd);
        if (ferror(infd)) {
            err = -3;
            break;
        }
        fwrite(buf, sizeof (unsigned char), len, outfd);
    }
    fclose(infd);
    fclose(outfd);
    return err;
}

