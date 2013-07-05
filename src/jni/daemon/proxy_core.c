#include "fctl.h"

#include <sys/wait.h>
#include <signal.h>
#include <limits.h>

#ifdef __linux__
#include <linux/netfilter_ipv4.h>
#endif

#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

static int np_run(struct fctl_proxy_t* proxy);
static  int __do_local_send(struct fctl_proxy_t* proxy, struct fctl_session_t *session);
static  int __do_remote_send(struct fctl_proxy_t* proxy, struct fctl_session_t *session);
static inline int __close_local(struct fctl_proxy_t*  proxy, struct fctl_session_t *session);
static inline int __close_remote(struct fctl_proxy_t* proxy, struct fctl_session_t *session);
static int proxy_connect_remote(struct fctl_proxy_t* proxy, struct fctl_session_t*session, int is_sync, 
        const char* remote_ip, int remote_port);
static int __do_connect_remote(struct fctl_proxy_t* proxy, struct fctl_session_t *session, 
        const char* remote_ip, int remote_port);

extern struct fctl_conf_t g_conf;
extern char g_configfile[MAX_FILE_NAME];

static fd_set  rset, wset , all_read_set, all_write_set;
static  int maxfd = 0;
static  time_t last_clear_timeout = 0;

#define PROXY_STATUS_UNKNOWN 0
#define PROXY_STATUS_UP      1
#define PROXY_STATUS_DOWN    2

static int proxy_status = PROXY_STATUS_UNKNOWN; 

int inline set_noneblock(int fd){
    int flags = 0;
	
    if (fd < 0 ){
        return -1;
    }
	
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0){
        return -2;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0 ){
        return -3;
    }

    return 0;
}

struct fctl_proxy_t* np_new(struct fctl_proto_t* proto, int port,
     int max_bind_port ,int min_bind_port){
    struct fctl_proxy_ops_t ops;
    struct fctl_proxy_t* proxy = NULL;
	
     proxy = proxy_new(proto, port, max_bind_port , min_bind_port);
    if (proxy == NULL)
        return NULL;

    proxy->run = np_run;
    proxy->local_send = __do_local_send;
    proxy->remote_send = __do_remote_send;
    proxy->local_close = __close_local;
    proxy->remote_close = __close_remote;
    proxy->remote_connect = __do_connect_remote;
    
    snprintf(proxy->worker_sock_path, sizeof(proxy->worker_sock_path), 
        "%s/%d", g_conf.spool_dir, port);
 			
    return proxy;	
}

#ifdef __linux__
int inline get_original_dest(int sockfd, struct sockaddr_in * addr , int *len){
    *len = sizeof(struct sockaddr_in);
    DBGLOG("linux get_original_dest");
    
    if(getsockopt(sockfd, SOL_IP, SO_ORIGINAL_DST, (struct sockaddr *)addr, (socklen_t*)len)){
        ERRLOG("get original destination fail.\n");
        return -1;   
    }
    return 0;
}

int inline __get_real_sport(FILE *stream,  int proxy_port ,int local_port){
    char pattern[50];
    char realport_str[20];
     char buf[256];
    int i = 0, found=0 , err = 0;
    
    if (stream == NULL) {
        return -1;
    }

    snprintf(pattern , sizeof(pattern), "sport=%d dport=%d", proxy_port, local_port);

    while ( fgets(buf , sizeof(buf), stream) ) {
       
        if ( strstr(buf , pattern) == NULL)
             continue;
        
        i = 0;
        char* startp= strstr(buf , "sport=");
        if (startp == NULL){
            err = -2;
            goto ret;
        }
    
        startp += 6;
      
        while (startp[i] != ' '){
            realport_str[i] = startp[i];
            i++;
        }
       
        realport_str[i] = '\0';
        
        found = 1;
        break;
    }

ret:
    if (found){
        DBGLOG("sport found (%s)", realport_str);
        return atoi(realport_str);
    }

    return -3;

}

int inline get_original_dest1(int local_sockfd, int proxy_port ,int local_port, struct sockaddr_in * addr , int *len){
    FILE *stream = NULL;
    char buf[256];
    char pattern[50];
    char pattern1[50];
    char realip_str[20];
    char realport_str[20];
    int i = 0, found = 0, err = 0;

    stream = fopen("/proc/net/ip_conntrack" , "r");
    if (stream == NULL) {
        ERRLOG("can not open /proc/net/ip_conntrack");
        return -1;
    }

    int real_sport = __get_real_sport(stream , proxy_port , local_port);

    rewind(stream);

    if (real_sport < 0){
        err = -4;
        goto ret;
    }
    DBGLOG("proxy_port= %d , local_port = %d", proxy_port , real_sport);

   
   snprintf(pattern , sizeof(pattern), "sport=%d dport=%d", proxy_port,  real_sport);
   snprintf(pattern1 , sizeof(pattern1), "dport=%d", proxy_port);

 
    while ( fgets(buf , sizeof(buf), stream) ) {
        
        if ( strstr(buf , pattern) == NULL)
            continue;

        char* startp = strstr(buf , "dst=");
        
        if (startp == NULL){
            err = -2;
            goto ret;
        }
       
        startp += 4;
        while (startp[i] != ' '){
            realip_str[i] = startp[i];
            i++;
        }
        realip_str[i] = '\0';
            
        i = 0;
        startp= strstr(buf , "dport=");
        if (startp == NULL){
            err = -3;
            goto ret;
        }
        
        startp += 6;
        while (startp[i] != ' '){
             realport_str[i] = startp[i];
             i++;
        }
        realport_str[i] = '\0';

        found = 1;
        break;
    }
    

ret:
    if ( found ){
        addr->sin_family = AF_INET;   
        addr->sin_addr.s_addr = inet_addr(realip_str);
        addr->sin_port =  htons(atoi(realport_str));
        //printf("!!!!!!!found ip=%s port=%s\n", realip_str, realport_str);
    }
    
    fclose(stream);            

    return err;
}
#endif

#ifdef __APPLE__
int inline get_original_dest(int sockfd, struct sockaddr_in * addr , int *len){
    *len = sizeof(struct sockaddr_in);
    DBGLOG("mac get_original_dest"); 
    if(getsockname(sockfd, (struct sockaddr *)addr, (socklen_t*)len)){
        ERRLOG("get original destination fail.\n");
        return -1;   
    }
    return 0;
}
#endif

int inline proxy_bind_unused_port( struct fctl_proxy_t* proxy , int real_servfd ){
   struct sockaddr_in tmpaddr;
   
   memset(&tmpaddr,0,sizeof(struct sockaddr_in));
   
   tmpaddr.sin_family = AF_INET;
   tmpaddr.sin_addr.s_addr = htonl (INADDR_ANY);
      
   int port = 0;
   for( ; ;){
       port = proxy->get_port(proxy);	 
	   tmpaddr.sin_port = htons (port);
       if (bind(real_servfd, (struct sockaddr*)&tmpaddr, sizeof(tmpaddr)) ==0 ){
            break;
       }
    }

    return port;
}

int proxy_create_remote_sock(struct fctl_proxy_t* proxy, struct fctl_session_t*session){
   
    int real_servfd = 0 , i = 1; 

    real_servfd = socket(AF_INET, SOCK_STREAM, 0);
   
    if (real_servfd < 0){
       
       ERRLOG("socket error");
       return -3;       
   
    } 
    
    session->remote_fd = real_servfd;

    setsockopt(real_servfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
   
    return real_servfd;
}

int proxy_connect(struct fctl_proxy_t* proxy, struct fctl_session_t*session, 
    struct sockaddr_in * realaddr){
    
    int sockfd , len;
    struct sockaddr_in real_servaddr;

    sockfd = session->local_fd;

    session->remote_dst_port = ntohs(realaddr->sin_port) ;
    memcpy(&session->remote_addr, &realaddr, sizeof(*realaddr));

    DBGLOG("starting connect remote");

    if ( connect( session->remote_fd, (struct sockaddr*) realaddr, 
        sizeof(struct sockaddr_in)) < 0){
	   
	   if (errno !=EINPROGRESS){
           DBGLOG("connect error:%s", strerror(errno));
	       DBGLOG("connect remote server fail!");
	       return -4;
       }
       DBGLOG("connect fail");
       session->remote_status = SESSION_CONNECTING;
    
    }else{
        DBGLOG("connect successfully");
       session->remote_status = SESSION_CONNECTED;
    }
   
    return 0;
}


static inline void set_socket_buf_len(int sock){
 
 	if (sock > 0){
 	    
		int optarg;
		int len = sizeof(optarg);

		optarg = RWBUFF_LEN;
		setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&optarg, sizeof(optarg));
		
		len = sizeof(optarg);
		optarg = RWBUFF_LEN;
		setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&optarg, sizeof(optarg));
		
	}
}

int proxy_connect_remote(struct fctl_proxy_t* proxy, struct fctl_session_t*session, int is_sync, 
        const char* remote_ip, int remote_port){
    struct sockaddr_in realaddr;

    int real_servfd = proxy_create_remote_sock(proxy,session);
    if (real_servfd < 0){
        ERRLOG("proxy_create_remote_sock error");
        return -1;
    }    
    if (!is_sync){
        if ( set_noneblock(real_servfd) < 0){
            ERRLOG("can not set socket none block");
            return -2;
        }
    }

    set_socket_buf_len(real_servfd);

    DBGLOG("start binding.............");
    int port = proxy_bind_unused_port(proxy , real_servfd);
    
    DBGLOG("bind port OK , port is %d", port);
    if (port < 0){
        return -3;
    }

    int len = sizeof(struct sockaddr_in);
  
    if (remote_ip != NULL){
        realaddr.sin_family = AF_INET;   
        realaddr.sin_addr.s_addr = inet_addr(remote_ip);
        realaddr.sin_port =  htons(remote_port);
    }else{
#ifdef __linux__
        DBGLOG("get original destination = %d",g_conf.get_original_destination);
        if (g_conf.get_original_destination == 1){
            if (get_original_dest(session->local_fd, &realaddr, &len) < 0){
                ERRLOG("get original destination fail.\n");
                return -5;   
            }
        }else{
            if ( get_original_dest1(session->local_fd, proxy->port , 
                        ntohs(session->local_addr.sin_port), &realaddr, &len) < 0 ){
                ERRLOG("get original destination fail.\n");
                return -5;   
            }
        }
#endif

#ifdef __APPLE__
        DBGLOG("Mac prepare to get destination!!!!!!!!!!");
        if (get_original_dest(session->local_fd, &realaddr, &len) < 0){
            ERRLOG("get original destination fail.\n");
            return -5;   
        }
#endif
    }
    
    //if ( get_original_dest1(session->local_fd, proxy->port , 
     //       ntohs(session->local_addr.sin_port), &realaddr, &len) < 0 ){
	/*if (get_original_dest(session->local_fd, &realaddr, &len) < 0){
        ERRLOG("get original destination fail.\n");
        return -5;   
    }*/
    
    unsigned long oriip = realaddr.sin_addr.s_addr;
    
    DBGLOG("original destionatin: %s(%x):%d\n", 
        inet_ntoa(realaddr.sin_addr), oriip, ntohs(realaddr.sin_port));    
	
    
    if ( (oriip & 0xFF) == 0x7F ){
        return -6;
    }

    DBGLOG("disable %d %d", port , ntohs(realaddr.sin_port));
    nat_disable_port(port, ntohs(realaddr.sin_port) );
    
    session->remote_src_port = port;

    DBGLOG("bind port %d",port);

    if ( proxy_connect(proxy , session, &realaddr) <  0 ){
       // close(session->local_fd);
        return -4;
    }
    
    FD_SET(session->remote_fd,&all_read_set);
	
    return 0;     
}

static inline int __close_local(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
   
    if (session->local_fd > 0){
        FD_CLR(session->local_fd, &all_write_set);
        FD_CLR(session->local_fd, &all_read_set);
        SAFE_CLOSE(session->local_fd);
        session->local_status = SESSION_CLOSED;
        session->closing_time =  time(0);
        session->local_fd = -1;
    }
      
    return 0;
}

static inline int __close_remote(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
   
    if (session->remote_fd > 0){
        FD_CLR(session->remote_fd, &all_write_set);
        FD_CLR(session->remote_fd, &all_read_set);
        SAFE_CLOSE(session->remote_fd);
        session->remote_status = SESSION_CLOSED;
        session->closing_time =  time(0);
        session->remote_fd = -1;
    }
      
    return 0;
}


static inline int __close_session(struct fctl_session_t *session){
   
    if (session->remote_fd > 0){
        FD_CLR(session->remote_fd, &all_write_set);
        FD_CLR(session->remote_fd, &all_read_set);
    }
    if (session->local_fd > 0){
        FD_CLR(session->local_fd, &all_write_set);
        FD_CLR(session->local_fd, &all_read_set);
    }
    
    session->close(session);
      
    return 0;
}

static int __do_connect_remote(struct fctl_proxy_t* proxy, struct fctl_session_t *session, const char* remote_ip, int remote_port)
{
    int ret = 0;
    if (proxy == NULL || session == NULL){
        return -1;
    }

    if ( proxy_connect_remote(proxy, session, 0, remote_ip, remote_port) ==0) {
        if (session->remote_fd > maxfd)
            maxfd = session->remote_fd;

        if (session->remote_status == SESSION_CONNECTED){
           ret =  __do_local_send(proxy, session);
        }else{
            FD_SET(session->remote_fd,&all_write_set);
        }

    }else{
        __close_session(session);
        ret =  -2;
    }
    return 0;
}

static inline  int __do_new_local_connection(struct fctl_proxy_t* proxy, int listenfd) {
    struct sockaddr_in cliaddr;	
    socklen_t  clilen = sizeof(cliaddr); 
	         
    int connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    
    if (connfd < 0){
        return -1;
    }         
    DBGLOG("accept a new connect src port=%d(%d) connfd=%d", ntohs(cliaddr.sin_port),cliaddr.sin_port, connfd);
             
    if ( connfd > 0 ){
        struct fctl_session_t * session = session_new(proxy, connfd,&cliaddr);
        list_add(&session->list , &proxy->session_head);

        proxy->proto->on_connect(session);

        FD_SET(connfd, &all_read_set);
        if (connfd > maxfd)
            maxfd = connfd;

        set_noneblock(connfd);
        set_socket_buf_len(connfd);
    }

    return 0;
}

static inline  int __do_notification(struct fctl_proxy_t* proxy, int listenfd) {
   /* 
    struct list_head *pos =  NULL;	  
    char buf[2048];
    socklen_t len = 0; 
    int n = 0, found = 0;
    struct sockaddr_un cliaddr;
     
    len = sizeof(struct sockaddr_un);
    n = recvfrom(listenfd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len);
     
    if (n <  sizeof(struct event_header_t)) {
        ERRLOG("work socket recvfrom error:%s", strerror(errno));
        return -1;   
    }
    struct event_header_t* header = (struct event_header_t*)buf;
    
    DBGLOG("session id = %d", header->session_id);
    DBGLOG("event type = %d", header->event_type);
    DBGLOG("event len = %d, receive len=%d", header->event_len, n);
     
    if (n < header->event_len){
        ERRLOG("worker socket error: size is %d ,but %d is required", n , header->event_len);
        return -3;
    }
     
     struct fctl_session_t *session;
     
     list_for_each(pos, &proxy->session_head) {
         session = (struct fctl_session_t *)pos;
         if (session->session_id == header->session_id) {
            found = 1;
            break;
        }
     }
     if (!found){
        ERRLOG("worker socket error: session %d did not be found!", header->session_id);
        return -2;
     }       
     
     if (proxy->proto->on_notification)
        proxy->proto->on_notification(session , header);
     
     return 0;
*/
    return 0;
}

static inline int __do_remote_connected(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
    int ret = 0;
     int error;
     socklen_t len = sizeof(error);       
     
     int remote_sockfd = session->remote_fd;

     getsockopt(remote_sockfd, SOL_SOCKET, SO_ERROR, &error , &len);
     
     if (error != 0 ){
        __close_session(session);
        DBGLOG("connect error!\n");
        return -1;

     }else{
     
         session->remote_status = SESSION_CONNECTED;   
         DBGLOG("connect remote successfully");

         FD_CLR(remote_sockfd, &all_write_set);
         
         ret =  __do_local_send(proxy, session);
         //proxy->proto->on_connect(session);
     }

     return ret;
}


static  int __do_local_send(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
    int local_sockfd = session->local_fd; 		 
    int remote_sockfd = session->remote_fd;
    int ret = 0;
    
    if (session->remote_status != SESSION_CONNECTED)
        return 0;

    ret =  session->on_local_send(session);

     if (ret < 0){
        DBGLOG("session local_send error!");
        __close_session(session);
        return -1;
    }else if (ret == 0){
        
        if (session->local_status != SESSION_CLOSING
           && session->local_status != SESSION_CLOSED) 
            FD_SET(local_sockfd , &all_read_set);
        
    }else {
       
        FD_CLR(local_sockfd , &all_read_set);
    }
                      
    if ( rwbuff_get_sendn(&session->local_buf) != 0 ){
                
           FD_SET(remote_sockfd, &all_write_set);
                  
    }else{
                          
         if (session->remote_status !=  SESSION_CONNECTING)
                FD_CLR(remote_sockfd, &all_write_set);
                       
    }
    
    if (session->local_status ==  SESSION_CLOSED){
        if (proxy->proto->proxy_type == PROXY_SYN)
            shutdown(remote_sockfd , SHUT_WR);
    }
    
    return 0;
}

static inline int __do_local_recv(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
    int local_sockfd = session->local_fd; 		 
    int remote_sockfd = session->remote_fd;
    int ret = 0;

    ret = session->on_local_recv(session) ;
    
    if (ret < 0){
        DBGLOG("on_local_recv error");
        __close_session(session);
        return -1;
    }
                  
    if (session->local_status ==  SESSION_CLOSING){
        FD_CLR(local_sockfd, &all_read_set);       
    }
     
    return __do_local_send(proxy , session);

}

static  int __do_remote_send(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
    int local_sockfd = session->local_fd; 		 
    int remote_sockfd = session->remote_fd;
    int ret = 0;
    
    ret =  session->on_remote_send(session);
    
    if (ret < 0){
        DBGLOG("on remote_send error");
         __close_session(session);
        return -1;    
    }else if (ret == 0){
        
        if (session->remote_status != SESSION_CLOSING
           && session->remote_status != SESSION_CLOSED) 
            FD_SET(remote_sockfd , &all_read_set);
        
    }else {
       
        FD_CLR(remote_sockfd , &all_read_set);
    }
             
    if ( rwbuff_get_sendn(&session->remote_buf) != 0 ){
                
        FD_SET(local_sockfd, &all_write_set);
                  
    }else{
                          
         FD_CLR(local_sockfd, &all_write_set);
    }
  
    if (session->remote_status ==  SESSION_CLOSED){
      
         shutdown(local_sockfd , SHUT_WR);
        // close(local_sockfd);
    }
   
    return 0;
}

static inline int __do_remote_recv(struct fctl_proxy_t* proxy, struct fctl_session_t *session){
    int local_sockfd = session->local_fd; 		 
    int remote_sockfd = session->remote_fd;
    int ret = 0;

    
    ret = session->on_remote_recv(session) ;
    
    
    if (ret < 0){
        DBGLOG("on_remote_recv error %d", ret);
        __close_session(session);
        return -1;
    }
                  
    if (session->remote_status ==  SESSION_CLOSING){
        FD_CLR(remote_sockfd, &all_read_set);       
    }
                  
    return __do_remote_send(proxy , session);

}

static void sig_alrm(int signo)
{
    destroy_iptables_nat_rules();
    ERRLOG("FortiProxy self test failed. Disable FortiProxy. ");
}

static void clear_timeout_sessions(struct fctl_proxy_t* proxy)
{
    struct list_head *pos =  NULL;	  

    if ((time(0) - last_clear_timeout) > 30){ 
        list_for_each(pos, &proxy->session_head) {
            struct fctl_session_t *session = (struct fctl_session_t *)pos;

            if ( session->remote_status == SESSION_CONNECTING &&  (time(0) - session->create_time) > 30){
                proxy->proto->on_disconnect(session);
                nat_enable_port(session->remote_src_port , session->remote_dst_port);
                __close_session(session);
                pos = session->list.prev;
                list_del(&session->list);
                session->free(session);
            }

        }
        last_clear_timeout = time(0);
    }
}

static int np_do_proxy(struct fctl_proxy_t* proxy){

    int listenfd = 0; // worker_listenfd = 0, 
    int nready = 0;
   
    //struct sockaddr_in cliaddr;	
    int clilen = 0 , ret = 0;	

   // FcckUpdateCookie();	
   // SiInitialize(g_conf.home_dir);

    INFOLOG("vvvvvvvvvvvvvvvvvvvv");
    listenfd = proxy->listen(proxy);
    if (listenfd < 0){
        ERRLOG("proxy can not start");
	 return -1;	
    }
    INFOLOG("00000000000000000vvvvvvvvv");
    
   /* worker_listenfd = proxy->worker_listen(proxy);
    INFOLOG("111111111111100000000000000000vvvvvvvvv");
    if (worker_listenfd < 0){
        ERRLOG("worker socket can not start");
	 return -2;	
    }*/
	
    INFOLOG("vvvvvvvvvvvvvvvvvvvv111111111111");
    if (set_noneblock(listenfd) < 0){
	    ERRLOG("socket can not be set none block.");	
        return  -2;
    }
    INFOLOG("vvvvvvvvvvvvvvvvvvvv22222222222222222");
		
    maxfd = listenfd; // > worker_listenfd ? listenfd : worker_listenfd;
	
    FD_ZERO(&all_read_set);
    FD_ZERO(&all_write_set);
    FD_SET(listenfd, &all_read_set);
   // FD_SET(worker_listenfd, &all_read_set);
  
    INFOLOG("vvvvvvvvvvvvvvvvvvvv333333333333333");
#if ENABLE_SELF_TEST
    //Proxy self test
    //When FortiClient starts, a program called proxytest runs . It connects to  a ip's 80 port.
    //So if our fiewall rules work fine , the proxy must receive a packet in 30 secs.
   // sig_t old_alarm = signal(SIGALRM, sig_alrm);
    //alarm(30);
#endif
    for ( ; ; ) {
         
         rset   = all_read_set;         
         wset = all_write_set;

         struct timeval tv;
         tv.tv_sec = 30;
         tv.tv_usec = 0;
         nready = select(maxfd + 1, &rset, &wset, NULL, &tv); 

         if ( nready <= 0 ){
             //ERRLOG("select: %s" , strerror(errno));
             clear_timeout_sessions(proxy);
             continue;
         }
#if ENABLE_SELF_TEST
      //  if (proxy_status == PROXY_STATUS_UNKNOWN){
      //      alarm(0);
      //      signal(SIGALRM, old_alarm);
      //      proxy_status = PROXY_STATUS_UP;
      //  }
#endif
         if (FD_ISSET(listenfd, &rset)) {  /* new client connection */
             
              __do_new_local_connection(proxy , listenfd);

            //  if (--nready <= 0)
            //       continue;          
            
     	 }
     	
#if 0
     	 if (FD_ISSET(worker_listenfd, &rset)) {  /* new client connection */
             
              __do_notification(proxy , worker_listenfd);

            //  if (--nready <= 0)
            //       continue;          
            
     	 }
     	 
#endif
	     struct list_head *pos =  NULL;	  
         
         int i = 0;
         list_for_each(pos, &proxy->session_head) {
              i++;  
              struct fctl_session_t *session = (struct fctl_session_t *)pos;
		    
		     if (FD_ISSET(session->remote_fd, &rset) 
                 || FD_ISSET(session->remote_fd, &wset)){
                
                if (session->remote_status == SESSION_CONNECTING){
                    
                    int err = __do_remote_connected(proxy , session);

                    if (--nready <= 0 || err < 0)
                        goto ret;
                }else{
                    if ( session->remote_fd > 0 && FD_ISSET(session->remote_fd, &wset)) {
               
                        int err = __do_local_send(proxy , session);

                         if (--nready <= 0 || err < 0)
                             goto ret;      
                    }

                               
                    if ( session->remote_fd > 0 && FD_ISSET(session->remote_fd, &rset)) {
                  
                        int err = __do_remote_recv(proxy, session);

                        if (--nready <= 0 || err < 0)
                             goto ret;    
                    }
                
                
                }
             }


              if (session->local_fd > 0  && FD_ISSET(session->local_fd, &rset)) {
                  
                   int err = __do_local_recv(proxy , session);

                  if (--nready <= 0 || err < 0)
                     goto ret;       
              }		 

                          
              if (session->local_fd > 0  && FD_ISSET(session->local_fd, &wset)) {
                  
                 __do_remote_send(proxy , session);

                //  if (--nready <= 0)
                 //     goto ret;       
              }
               
   ret:
            
              if (session->is_closed(session)){
                  DBGLOG("a session end....................");
                  
                  proxy->proto->on_disconnect(session);
                  
                  nat_enable_port(session->remote_src_port , session->remote_dst_port);
                  
                  __close_session(session);
                  
                  pos = session->list.prev;
                  list_del(&session->list);
                  session->free(session);
                  
              }  
	       
        }
        DBGLOG("total sessions %d", i);
		  
        clear_timeout_sessions(proxy);
    }
}

static void sig_reload_config(int signo)  
{
 /*   DBGLOG("reload configuration");
    load_conf(&g_conf , g_configfile);  
    log_init(g_conf.log_level, FCTLOG_SOURCE_PROXY , g_conf.avlogd_sock);

    cfg_get_av_sig_version(g_conf.av_signature_version);
    cfg_get_av_engine_version(g_conf.av_engine_version, sizeof(g_conf.av_engine_version), NULL);

    struct fct_settings settings; 
    fctsetting_init(&settings);
    fctsetting_load(&settings, FCT_DEFAULT_SETTING_FILE);
    g_conf.av_enable_real_time  =  settings.rtscan_enable; 
    fctsetting_destroy(&settings);

    //Signaure may be updated , so the binary cookie need to be updated too.
    FcckUpdateCookie();*/
}

static void  sig_int(int signo){
    INFOLOG("proxy ended.")
    exit(0);
}

static int np_run(struct fctl_proxy_t* proxy){
    pid_t childpid = 0;
    #ifdef __linux__
        __sighandler_t err;
    #endif
    #ifdef __APPLE__
        sig_t err;
    #endif
    
   if ( (childpid = fork()) == 0) { 
       FILE* fp = fopen("/tmp/proxy1.log", "a+");
       if (fp){
           fprintf(fp, "%s\n", "12345");
           fclose(fp);
        }
       //log_init(20, 1, "/tmp/proxy.log");
       printf("dddddddd\n");    
#ifdef __linux__
      // av_register_trusted_module(NULL);
#endif
      err = signal(SIGHUP , sig_reload_config);
      if (err ==  SIG_ERR){
        ERRLOG("can not install the signal  handler for SIGHUP");
      }   
      signal(SIGINT, sig_int);
      signal(SIGTERM, sig_int);
      signal(SIGCHLD, SIG_IGN);
      sig_reload_config(-1);
      np_do_proxy(proxy);
	  exit(0);	 
    }else{
        printf("child pid=%d\n", childpid);
      proxy->pid = childpid;
    }
       
   return 0;
}



/*
int main(void) {
    int i;  
    WorkQueue *wq = wq_create(4, 10000);
    if (!wq){
        printf("WorkQueue can not create!\n");
        return 1;
    }    
    for(i = 0; i < 10000 ; i++){
        wq_future_do(wq , temp_do , NULL); 
    }
    sleep(2);
    wq_destroy(wq);

}
*/

