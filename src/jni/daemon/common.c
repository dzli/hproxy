#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "forticlient.h"
#include "conf.h"
//#include "log.h"


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
	
	LOAD_STRING(pconf , "home_dir" , conf->home_dir , MAX_FILE_NAME);
	LOAD_STRING(pconf , "conf_file" , conf->conf_file , MAX_FILE_NAME);
	LOAD_STRING(pconf , "log_dir" ,   conf->log_dir , MAX_FILE_NAME);
	LOAD_STRING(pconf , "log_file" ,   conf->log_file, MAX_FILE_NAME);
	LOAD_STRING(pconf , "pid_file" ,   conf->pid_file , MAX_FILE_NAME);
	LOAD_STRING(pconf , "av_service" ,   conf->av_service , MAX_FILE_NAME);
	LOAD_STRING(pconf , "avlog_service" ,   conf->avlog_service , MAX_FILE_NAME);
	LOAD_STRING(pconf , "avrt_service" ,   conf->avrt_service , MAX_FILE_NAME);
	LOAD_STRING(pconf , "avcrt_service" ,   conf->avcrt_service , MAX_FILE_NAME);
	LOAD_STRING(pconf , "clamscan" ,   conf->clamscan , MAX_FILE_NAME);
	LOAD_STRING(pconf , "freshclam" ,   conf->freshclam , MAX_FILE_NAME);
    LOAD_STRING(pconf , "clamd_sock" ,   conf->clamd_sock , MAX_FILE_NAME);
	LOAD_STRING(pconf , "spool_dir" ,   conf->spool_dir , MAX_FILE_NAME);
    LOAD_STRING(pconf , "iptables" ,   conf->iptables , MAX_FILE_NAME);
    LOAD_STRING(pconf , "avlogd_sock" ,   conf->avlogd_sock , MAX_FILE_NAME);
    LOAD_STRING(pconf , "avlog_file" ,   conf->avlog_file , MAX_FILE_NAME);
    LOAD_STRING(pconf , "avlisten_sock" ,   conf->avlisten_sock , MAX_FILE_NAME);
    LOAD_STRING(pconf , "av_quarantine_dir" ,   conf->av_quarantine_dir , MAX_FILE_NAME);
    LOAD_STRING(pconf , "fctdb_file" ,   conf->fctdb_file , MAX_FILE_NAME);
    LOAD_STRING(pconf , "av_sig_dir" ,   conf->av_sig_dir , MAX_FILE_NAME);
    
    LOAD_INT(pconf , "get_original_destination", conf->get_original_destination);
    LOAD_INT(pconf , "log_level", conf->log_level);
    LOAD_STRING(pconf , "serial_number" , conf->serial_number, sizeof(conf->serial_number));
    LOAD_STRING(pconf , "UID" , conf->UID, sizeof(conf->UID));
    LOAD_INT(pconf , "enable_proxy", conf->enable_proxy);
    LOAD_INT(pconf , "expired_date", conf->expiry_date);
    
	LOAD_INT(pconf , "enable_http" , conf->enable_http);
	LOAD_INT(pconf , "http_proxy_listen_port" , conf->http_proxy_listen_port);
	LOAD_INT(pconf , "http_min_bind_port" , conf->http_min_bind_port);
	LOAD_INT(pconf , "http_max_bind_port" , conf->http_max_bind_port);
    LOAD_STRING(pconf , "http_port" , ports , sizeof(ports));
    split_ports(conf->http_ports, ports);
    
	LOAD_INT(pconf , "enable_ftp" , conf->enable_ftp);
	LOAD_STRING(pconf , "ftp_port" , ports , sizeof(ports));
    split_ports(conf->ftp_ports, ports);
	LOAD_INT(pconf , "ftp_proxy_listen_port" , conf->ftp_proxy_listen_port);
	LOAD_INT(pconf , "ftp_min_bind_port" , conf->ftp_min_bind_port);
	LOAD_INT(pconf , "ftp_max_bind_port" , conf->ftp_max_bind_port);
	
	LOAD_INT(pconf , "enable_pop3" , conf->enable_pop3);
	LOAD_STRING(pconf , "pop3_port" , ports , sizeof(ports));
    split_ports(conf->pop3_ports, ports);
	LOAD_INT(pconf , "pop3_proxy_listen_port" , conf->pop3_proxy_listen_port);
	LOAD_INT(pconf , "pop3_min_bind_port" , conf->pop3_min_bind_port);
	LOAD_INT(pconf , "pop3_max_bind_port" , conf->pop3_max_bind_port);

	LOAD_INT(pconf , "enable_smtp" , conf->enable_smtp);
	LOAD_STRING(pconf , "smtp_port" , ports , sizeof(ports));
    split_ports(conf->smtp_ports, ports);
	LOAD_INT(pconf , "smtp_proxy_listen_port" , conf->smtp_proxy_listen_port);
	LOAD_INT(pconf , "smtp_min_bind_port" , conf->smtp_min_bind_port);
	LOAD_INT(pconf , "smtp_max_bind_port" , conf->smtp_max_bind_port);

    LOAD_INT(pconf , "av_service_port" , conf->av_service_port);
    LOAD_STRING(pconf , "av_engine_version" ,   conf->av_engine_version , MAX_VERSION_LEN);
	LOAD_STRING(pconf , "av_signature_version" ,   conf->av_signature_version , MAX_VERSION_LEN);
	LOAD_INT(pconf , "av_enable_real_time" , conf->av_enable_real_time);
	LOAD_INT(pconf , "av_enable_pop3" , conf->av_enable_pop3);
	LOAD_INT(pconf , "av_enable_smtp" , conf->av_enable_smtp);
	LOAD_INT(pconf , "av_virus_fate" , conf->av_virus_fate);
	LOAD_INT(pconf , "av_real_time_virus_fate" , conf->av_real_time_virus_fate);
	
	LOAD_STRING(pconf , "av_signature_path", conf->av_signature_path,MAX_FILE_NAME);
    LOAD_STRING(pconf , "av_update_server", conf->av_update_server,sizeof(conf->av_update_server));
    LOAD_STRING(pconf , "av_update_time", conf->av_update_time, sizeof(conf->av_update_time));
	LOAD_INT(pconf , "av_cache_max_dir" , conf->av_cache_max_dir);
	LOAD_INT(pconf , "av_cache_max_file" , conf->av_cache_max_file);
	LOAD_INT(pconf , "av_max_open_scan_file" , conf->av_max_open_scan_file);
	LOAD_INT(pconf , "av_real_time_valid" , conf->av_real_time_valid);
	LOAD_INT(pconf , "av_rtscan_instance" , conf->av_rtscan_instance);
	LOAD_INT(pconf , "av_backendscan_instance" , conf->av_backendscan_instance);
	
    LOAD_INT(pconf , "av_enable_grayware" , conf->av_enable_grayware);
	LOAD_INT(pconf , "av_enable_rt_grayware" , conf->av_enable_rt_grayware);
    LOAD_INT(pconf , "av_enable_heuristic" , conf->av_enable_heuristic);
	LOAD_INT(pconf , "av_enable_rt_heuristic" , conf->av_enable_rt_heuristic);
	LOAD_INT(pconf , "av_enable_compress_scan" , conf->av_enable_compress_scan);
	LOAD_INT(pconf , "av_compress_limit" , conf->av_compress_limit);
	LOAD_INT(pconf , "av_enable_rt_compress_scan" , conf->av_enable_rt_compress_scan);
	LOAD_INT(pconf , "av_rt_compress_limit" , conf->av_rt_compress_limit);
	LOAD_INT(pconf , "av_rt_broadcast_console" , conf->av_rt_broadcast_console);
	LOAD_INT(pconf , "av_rt_alert_to_gui" , conf->av_rt_alert_to_gui);

    snprintf(conf->trust_file_cache, sizeof(conf->trust_file_cache),"%s/trust_cache", conf->av_sig_dir);

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

/*
 *	file descriptor printf()
*/

int fdprintf(int fd, char* format, ...)
{
	va_list ap;
	char buf[1024];
	int size, res, pos;

	va_start(ap, format);
	size = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (size == -1 || size > sizeof(buf)) {
		errno = ENOMEM;
		return -1;
	}

	for (pos = 0;;) {
/*
		if (timedout) {
			errno = ETIMEDOUT;
			return -1;
		}
*/
		if ((res = write(fd, buf+pos, size-pos)) == -1) {
			if (errno == EINTR) continue;
//			if (errno == ECONNRESET) return -1;
			return -1;
		}

		if (res == 0) break;

		pos += res;
		if (pos == size) break;
	}

	return res;
} /* fdprintf() */

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

