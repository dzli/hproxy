#ifndef __FCTL_H__
#define __FCTL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include "list.h"
#include "log.h"
#include "rwbuff.h"
#include "forticlient.h"
#include "cache.h"

//#define HTTP_RECV_ALL_TRAFFIC 1

//#define MAX_PORT  65536
//#define MIN_PORT  61001
//#define SESSION_BUF_LEN 32768

#define PROTO_NAME_LEN 20
#define MAX_PROXY_SIZE 10
#define SAFE_CLOSE(x)	if (x > 0)	{while (close(x) == -1 && errno == EINTR);}

#define SESSION_TIMEOUT  60

typedef enum{
	SESSION_INITIALIZED =0,
	SESSION_CONNECTING =1,
	SESSION_CONNECTED,
	SESSION_CLOSING,
	SESSION_CLOSED,
} SESSION_STATUS;

typedef enum{
	PROTO_SEND =0,
	PROTO_STOLEN =1,
	PROTO_RESERVED,
	PROTO_DROP
	
} PROTO_ACTION;

typedef enum{
    PROXY_SYN = 0,
    PROXY_ASY = 1
}PROXY_TYPE;

struct fctl_session_t;

struct event_header_t{
    int session_id;
    int event_type;
    int event_len;
};

struct event_av_result_t{
    struct event_header_t header;
    int  result;
    char cooked_file_name[MAX_FILE_NAME];  
    char description[1024];
};

/*
    protocol handler struct and functions
*/
struct fctl_proto_t{
    char proto_name[PROTO_NAME_LEN];
    int port;
    PROXY_TYPE proxy_type;
    int  (*on_local_rev)(struct fctl_session_t* , char*  , int);
    int  (*on_remote_rev)(struct fctl_session_t* , char*  , int );
    int  (*on_connect)(struct fctl_session_t* );
    int  (*on_disconnect)(struct fctl_session_t* );	
    int  (*on_notification)(struct fctl_session_t*, struct event_header_t* );
};
struct fctl_proto_t * http_new(void);
struct fctl_proto_t * ftp_new(void);
struct fctl_proto_t * pop3_new(void);
struct fctl_proto_t * smtp_new(void);


/*
   proxy struct and functions
*/
struct fctl_proxy_t{
    int daemon;
    int port;	
    pid_t pid;
    struct fctl_proto_t* proto;	
    //pthread_mutex_t mutex;
    //WorkQueue *wq ;	
    int valid_port;	 
     
    int max_bind_port;
    int min_bind_port;
    
    char proto_name[PROTO_NAME_LEN];
    
    unsigned int cur_session_id;
    char worker_sock_path[MAX_FILE_NAME];

    struct list_head	session_head;
	
    int  (*run)(struct fctl_proxy_t*);		
    int  (*get_port)(struct fctl_proxy_t*);	
    int  (*read)(int  , char* , int);
    int  (*write)(int  , char*  , int );
    int  (*listen)(struct fctl_proxy_t*);
    int  (*worker_listen)(struct fctl_proxy_t*);	
    int  (*local_send)(struct fctl_proxy_t* , struct fctl_session_t *);
    int  (*remote_send)(struct fctl_proxy_t* , struct fctl_session_t *);
    int  (*local_close)(struct fctl_proxy_t* , struct fctl_session_t *);
    int  (*remote_close)(struct fctl_proxy_t* , struct fctl_session_t *);
    int  (*remote_connect)(struct fctl_proxy_t* , struct fctl_session_t *, const char*, int);
    int  (*send_event)(struct fctl_proxy_t* ,struct event_header_t*);
};
struct fctl_proxy_ops_t{
    int  (*run)(struct fctl_proxy_t*);
    int  (*get_port)(struct fctl_proxy_t*);
    int  (*read)(int fd , char* buf , int buflen);
    int  (*write)(int fd , char* buf , int buflen);	
};

struct fctl_proxy_t* proxy_new(struct fctl_proto_t* proto, int port, int max_bind_port ,int min_bind_port);
int proxy_get_valid_port(struct fctl_proxy_t* proxy);
struct fctl_proxy_t* np_new(struct fctl_proto_t* proto, int port, int max_bind_port ,int min_bind_port);

/*
    spool struct and functions
*/
struct fctl_spool_t{
    int fd;
    char name[MAX_FILE_NAME];
    int size;
    int exists;	
};

void spool_close(struct fctl_session_t *session);
void spool_remove(struct fctl_session_t *session);
int spool_create(struct fctl_session_t *session);


/*
    session struct and functions
*/
struct fctl_session_t{
    struct list_head list;
	
    struct fctl_proxy_t* proxy;	

    int local_fd;
    int remote_fd;	

    struct sockaddr_in local_addr;

    int remote_src_port;
    int remote_dst_port;	
    struct sockaddr_in remote_addr;
	
    struct rwbuff_t local_buf;
    struct rwbuff_t remote_buf;

    int remote_status;
    int local_status;
    int proto_status;
    
    int local_retry_times;
    int remote_retry_times;
    
    void *private_data;

    time_t create_time;
    int  closing_time;

    struct fctl_spool_t spool;
    
    int session_id;
	
    int  (*on_local_recv)(struct fctl_session_t* );
    int  (*on_remote_recv)(struct fctl_session_t*);
    int  (*on_local_send)(struct fctl_session_t* );
    int  (*on_remote_send)(struct fctl_session_t*);
    int  (*on_local_close)(struct fctl_session_t* );
    int  (*on_local_connect)(struct fctl_session_t* );
    int  (*on_remote_close)(struct fctl_session_t* );
    int  (*on_remote_connect)(struct fctl_session_t* );
    int  (*is_closed)(struct fctl_session_t* );
    int  (*close)(struct fctl_session_t* );
    int  (*free)(struct fctl_session_t*); 	
};

struct fctl_session_t* session_new(struct fctl_proxy_t* proxy , int fd, struct sockaddr_in *local_addr);


/*
    proxy container struct and functions
*/
struct other_process_t{
    pid_t pid;
    int daemon;
    char filename[MAX_FILE_NAME];
    char arg1[1024];
};

struct proxy_container_t{
    int index;
    struct fctl_proxy_t* proxies[MAX_PROXY_SIZE];
    int others_count;
    struct other_process_t others[64];
};
int pc_restart_proxy(struct proxy_container_t* pc, pid_t proxy_pid);
int pc_run_all(struct proxy_container_t* pc);
int pc_kill_all(struct proxy_container_t* pc, int signo);
int pc_add_proxy(struct proxy_container_t* pc, struct fctl_proxy_t* proxy);
int pc_add_others(struct proxy_container_t* pc, int daemon,  char* filename, char* arg1);
int pc_start_others(struct proxy_container_t* pc, char* filename );
int pc_stop_others(struct proxy_container_t* pc, char* filename );
int pc_restart_others(struct proxy_container_t* pc, pid_t pid );
int pc_remove_pid(struct proxy_container_t* pc, pid_t proxy_pid);


/*
    nat functions
*/
int nat_disable_port(int src_port , int dst_port );
int nat_enable_port(int src_port , int dst_port );
void init_iptables_nat_rules();
void create_iptables_nat_rule(int dest_port , int redirect_port);
void destroy_iptables_nat_rules();

/*
    forticient check  functions
*/
int UpdateCookie(char* cookie, int buf_len);

#endif

