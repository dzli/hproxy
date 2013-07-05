#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#ifdef  __APPLE__
#include <sys/utsname.h>
#endif

#include "fctl.h"

void daemonize();
static void usage(char *filename);
static void start();
int write_pid(char* pidfile);

static void sig_term(int signo);
void  sig_chld(int signo);

static volatile int stop = 0;
struct proxy_container_t proxies;
struct fctl_conf_t g_conf;
static int service_status = 1;

char g_configfile[MAX_FILE_NAME];
static int g_foregound = 0;
int g_log_level = -1;

//static int _handle_cmd(struct fctrootcmd* cmd, void* arg);

void _on_exit()
{
   // FSM_destroy(FCT_GLOBAL_DATA_SHARE_KEY, sizeof(struct fct_global_data));
}

int main(int argc  , char* argv[]){
    
    int err = 0 , found_config = 0 , c = 0;
    char* cmd = NULL;
  
    extern char * optarg;
    extern int optind;
    const char *short_options = "hvc:l:f";

    static  struct option long_options [] ={ 
	    {"help",        0,		NULL,	'h' }, 
	    {"version",     0,		NULL,	'v' },
	    {"config",		1,		NULL,	'c' },
	    {"log",	    	1,		NULL,	'l' },
	    {"foreground",  0,		NULL,	'f' },
	    {NULL,			0,		NULL,	0   }
    };
 

    memset(&proxies, 0 , sizeof(proxies));

    //This buffer is shared by all processes , which used to exchange data.
    //if (FSM_init(FCT_GLOBAL_DATA_SHARE_KEY, sizeof(struct fct_global_data)) != 0)
    //    ERRLOG("init global share data error.");

   /* 
        forticlient default configuration file
    */
    strcpy(g_configfile , DEFAULT_CONF_FILE);
      
    while ((c=getopt_long(argc, argv, short_options,long_options,NULL)) != -1){
        switch (c){
        case 'v':
            printf("forticlient proxy  %s\n", "V3.0.0");
            exit(0);
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        case 'c':
            found_config  = 1;
            strcpy(g_configfile , optarg);
            break;
        case 'f':
		    g_foregound  = 1;
            break;	
        case 'l':
            g_log_level = atoi(optarg);	
            break;		
        default:
            usage(argv[0]);
            exit(-2);
        }
    }
	
	/*
	    advance past the arguments that were processed 
	    by getopt_long()
	*/  
    argc -= optind;	
    argv += optind;	
    
    if (argc > 0){
        cmd = argv[0];
        --argc; ++argv;
    }
	
    if (cmd == NULL){
        usage(argv[0]);
        exit(-2);
    }

    err = load_conf(&g_conf , g_configfile);
	
    if (err){
        ERRLOG("read config file %s error!", g_configfile);
        return -3;
    }
    if (g_log_level < 0)
        g_log_level = g_conf.log_level;

#ifdef __linux__
    if ( g_foregound )
        log_init_ex(g_log_level, FCTLOG_SOURCE_PROXY , g_conf.avlogd_sock, FCTLOG_TO_CONSOLE);
    else
        log_init(g_log_level, FCTLOG_SOURCE_PROXY , g_conf.avlogd_sock);
#endif

    if ( strcasecmp(cmd , "start") == 0){
	    
        if (!g_foregound)
	        daemonize();
        printf("pid file: %s\n", g_conf.pid_file);
	    err = write_pid(g_conf.pid_file);
	    
	    if ( err == 0 ){
	   
	       start();
           return 0;
	    
        }else if ( err > 0){
	        ERRLOG("forticlient proxy has already run!");
        }else{
	        return -4;
        }
	}
	
    if ( strcasecmp(cmd , "stop") == 0){
        int pid = get_pid(g_conf.pid_file);
        if (pid > 0){
            DBGLOG("pid = %d" , pid);
            kill(pid , SIGTERM);
            //unlink(g_conf.pid_file);
        }
    };
	
    if ( strcasecmp(cmd , "reload") == 0){
        int pid = get_pid(g_conf.pid_file);
        DBGLOG("pid = %d" , pid);
        kill(pid , SIGHUP);
    };
    
  	return 0;
}

static void sig_reload_config(int signo){
    load_conf(&g_conf , g_configfile);
    
    log_init(g_conf.log_level, FCTLOG_SOURCE_PROXY , g_conf.avlogd_sock);
    
   /* if (g_conf.av_enable_real_time){
        pc_start_others(&proxies , g_conf.avrt_service);
    }else{
        pc_stop_others(&proxies , g_conf.avrt_service);
    }*/
    
   pc_kill_all(&proxies, SIGHUP);
}

#define CREATE_PROXY(PROTO) if (g_conf.enable_##PROTO){\
        struct fctl_proto_t* PROTO##_proto = PROTO##_new();\
        struct fctl_proxy_t* PROTO##_proxy = np_new( PROTO##_proto,\
            g_conf.PROTO##_proxy_listen_port,\
            g_conf.PROTO##_max_bind_port , g_conf.PROTO##_min_bind_port);\
        pc_add_proxy(&proxies , PROTO##_proxy);\
        int i = 0;\
        while(g_conf.PROTO##_ports[i] != 0){\
            create_iptables_nat_rule(g_conf.PROTO##_ports[i] , g_conf.PROTO##_proxy_listen_port);\
            i++;\
        }\
    }

static void start(){

#ifdef  __APPLE__
    const char    *kextname;
    struct  utsname un;
#endif

    signal(SIGPIPE , SIG_IGN );	
    signal(SIGHUP , sig_reload_config);
 

	atexit(_on_exit);

#ifdef __APPLE__
    if ( g_foregound )
        log_init_ex(g_log_level, FCTLOG_SOURCE_PROXY , FCT_DEFAULT_LOG_FILE , FCTLOG_TO_CONSOLE);
    else
        log_init(FCTLOG_LEVEL_INFO, FCTLOG_SOURCE_PROXY , FCT_DEFAULT_LOG_FILE);
#endif
  
    memset(&proxies ,0 , sizeof(proxies));

    printf("11111\n");
    if (g_conf.enable_proxy){
        init_iptables_nat_rules();
        CREATE_PROXY(http);
       // CREATE_PROXY(ftp);
       // CREATE_PROXY(pop3);
       // CREATE_PROXY(smtp);
    }

	char service[MAX_FILE_NAME];
#if 0	
#ifdef __APPLE__
	signal(SIGCHLD, SIG_IGN);
    if (uname(&un) < 0) {
        un.release[0] = '9';
        un.release[1] = '\0';
    }
    if ((un.release[0] == '1') &&
      (un.release[1] == '0') && (un.release[2] == '.')) {
        /* Snow leopard */
        kextname = "avkern_sl.kext";
    } else {
        kextname = "avkern.kext";
    }
	snprintf(service, sizeof(service),
            "kextload %s/module/%s", g_conf.home_dir, kextname);
	system(service);
	sleep(2);
	//bypass_rtscan();
	
    char osver[256];
    mac_get_os_version(osver, sizeof(osver));
	snprintf(service, sizeof(service),"%s/bin/fctavrt", g_conf.home_dir);
    // now , two instances of fctavrt run.
    pc_add_others(&proxies, DAEMON_RT , service, NULL);
    pc_add_others(&proxies, DAEMON_RT, service, NULL);
    pc_add_others(&proxies, DAEMON_RT, service, NULL);
    snprintf(service, sizeof(service),"%s/bin/fctavms", g_conf.home_dir);
    pc_add_others(&proxies, DAEMON_MS , service, NULL);
#define MAC_OS_BUG_VERSION "10.6"
    if(strstr(osver, MAC_OS_BUG_VERSION)){
        snprintf(service, sizeof(service),"%s/bin/proxytest", g_conf.home_dir);
        pc_add_others(&proxies, DAEMON_PT , service, NULL);
    }
	
#endif

#ifdef __linux__
    pc_add_others(&proxies, DAEMON_MS ,g_conf.av_service, NULL);
    pc_add_others(&proxies, DAEMON_RT, g_conf.avcrt_service, NULL);
    
    snprintf(service, sizeof(service),"%s/bin/fct_autoupdate", g_conf.home_dir);
    pc_add_others(&proxies, DAEMON_AUTO_UP, service, NULL);

    //pc_add_others(&proxies, g_conf.avlog_service);
  
  //  if (g_conf.av_enable_real_time){
        int i = 1;
        for(; i <= g_conf.av_rtscan_instance; i++){
            char num[64];
            snprintf(num ,sizeof(num),"%d", i);
            pc_add_others(&proxies, DAEMON_RT, g_conf.avrt_service, num);
        }
  //  }
    
#endif    
 #endif
  
    pc_run_all(&proxies);
    
    signal(SIGTERM , sig_term);
    signal(SIGINT, sig_term);
    signal(SIGCHLD, sig_chld);
     
   	while(!stop)
   		pause();  
    printf("pid= %d\n", getpid());
    //handle_cmd_request(_handle_cmd, NULL, g_conf.home_dir);

   	pc_kill_all(&proxies, SIGTERM);

    unsigned int unsleep = 2;
    while(unsleep > 0){
        unsleep = sleep(unsleep); 
    }
 
    pc_kill_all(&proxies, SIGKILL);

    unsleep = 1;
    while(unsleep > 0){
        unsleep = sleep(unsleep); 
    }

   	INFOLOG(" FortiClient stopped.\n");
   	
   	unlink(g_conf.pid_file);

    if (g_conf.enable_proxy){
        destroy_iptables_nat_rules();   	
    }
    
	//snprintf(service, sizeof(service),
        //    "kextunload %s/module/%s", g_conf.home_dir, kextname);
	//system(service);
	sleep(1);
}
/*
static int _handle_cmd(struct fctrootcmd* cmd, void* arg)
{

    if (cmd == NULL)
        return 0;
    switch (cmd->type){
	
        case FCTROOT_CMD_START:
            if (service_status == 1)
                return 0;
            pc_run_all(&proxies);
            service_status = 1;
			break;
    	
        case FCTROOT_CMD_STOP:
            stop = 1;
            break;

        case FCTROOT_CMD_QUERY_STOP:
            printf("stop=%d\n",stop);
            return stop;

#ifdef __APPLE__
        case FCTROOT_CMD_REALOD_SETTINGS:
             pc_kill_all(&proxies, SIGHUP);
            break;
#endif

    }

    return 0;
}
*/
/*
int lockfile(int fd){
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
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
        return -1;
    }
    
    fscanf(stream, "%d", &pid);
    fclose(stream);
    
    return pid;

}
*/
static void usage(char *filename){
	printf("Usage: fctproxyd [options] {start|stop|reload}\n");
	printf("options:\n");
    printf("--version -v       :  version information.\n");
    printf("--config -c file   :  configuration file.\n");
    printf("--foreground -f    :  run on foreground, default run as daemon.\n");
    printf("--log -l {1|2|3|4} : \n");
    printf("          1: output error mesage.\n");
    printf("          2: output error and warning mesage.\n");
    printf("          3: output error , warning and info mesage.\n");
    printf("          3: output all mesages including debug.\n");
}

static void sig_term(int signo)  
{
  //  printf("proxy is going to be shutdown.");
   // pc_kill_all(&proxies, SIGINT);
    stop = 1;
   // destroy_iptables_nat_rules();
}

void  sig_chld(int signo){
    pid_t    pid;
    int      stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0){
        printf("daemon %d terminated.\n", pid);
        INFOLOG("proxy %d terminated", pid);
        if (!stop)
            pc_restart_others(&proxies, pid);
        else
            pc_remove_pid(&proxies, pid);
    } 
   
    return;
}

void daemonize(){
    int                 i;
    pid_t               pid;
    struct rlimit       rl;
    struct sigaction    sa;
    /*
     * Clear file creation mask.
     */
    umask(0);

    /*
     * Get maximum number of file descriptors.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0){
        ERRLOG("can't get file limit");
        exit(-1);
    }
    
    /*
     * Become a session leader to lose controlling TTY.
     */
    if ((pid = fork()) < 0){
        ERRLOG("can't fork");
        exit(-2);
    }
    else if (pid != 0) /* parent */
        exit(0);
    
    setsid();

    /*
     * Ensure future opens won't allocate controlling TTYs.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0){
        ERRLOG("can't ignore SIGHUP");
        exit(-3);
    }
    
    if ((pid = fork()) < 0){
        ERRLOG("can't fork");
        exit(-2);
    }
    else if (pid != 0) /* parent */
        exit(0);

   
    /*
     * Close all open file descriptors.
     */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    /*
     * Attach file descriptors 0 to /dev/null.
     */
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDONLY);
    //open(g_conf.log_file, O_CREAT|O_RDWR|O_TRUNC);
    //open(g_conf.log_file, O_CREAT|O_RDWR|O_TRUNC);
    
    //fd0 = open(g_conf.log_file, O_CREAT|O_RDWR|O_TRUNC);
    //fd1 = dup(0);
    //fd2 = dup(0);
    
}

