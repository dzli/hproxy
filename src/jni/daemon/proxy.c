#include <netinet/in.h>
#include <sys/socket.h> 
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#include "fctl.h"

static int proxy_read(int fd, char* buf , int buflen){
    return read(fd , buf , buflen);
}

static int proxy_write(int fd, char* buf , int buflen){
    return write(fd , buf , buflen);
}


int proxy_get_valid_port(struct fctl_proxy_t* proxy){
    int port = 0;

    if  (proxy->valid_port == proxy->min_bind_port)
		proxy->valid_port = proxy->max_bind_port;
    proxy->valid_port--;
   
    return proxy->valid_port;	
}

static int proxy_listen(struct fctl_proxy_t* proxy){

    int listenfd = 0;
    struct sockaddr_in servaddr;
   
   
    if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0){
        ERRLOG("%s proxy socket error: %s", strerror(errno));
        return( -1 ); 
    }
   

    int i = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	//setsockopt(listenfd, SOL_SOCKET, SO_LINGER,
	//			   &linger_opt, sizeof(linger_opt));
				   
    memset(&servaddr,0,sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (proxy->port);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
	    ERRLOG("%s proxy bind error: %s", strerror(errno));
	    return(-2);
    }

  
    if (listen(listenfd, SOMAXCONN) <0 ){
	    ERRLOG("%s proxy listen error: %s", strerror(errno));
	    return(-3);
    }

    INFOLOG("%s proxy start and listen on port %d", proxy->proto->proto_name, proxy->port);
	
    return listenfd;	
}

int proxy_worker_listen(struct fctl_proxy_t* proxy){

    int listenfd = 0;
    socklen_t len = 0;
    struct sockaddr_un  servaddr , addr2;
   
    if((listenfd= socket(AF_LOCAL, SOCK_DGRAM, 0))<0){
        ERRLOG("%s worker socket error: %s", strerror(errno));
        return( -1 ); 
    }
   
    unlink(proxy->worker_sock_path);
  			   
    memset(&servaddr,0,sizeof(struct sockaddr_un));
    servaddr.sun_family = AF_LOCAL;
    strncpy(servaddr.sun_path, proxy->worker_sock_path, sizeof(servaddr.sun_path) - 1);

    
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
	    ERRLOG("%s worker socket bind error: %s", strerror(errno));
	    return(-2);
    }

    len = sizeof(struct sockaddr_un);
    getsockname(listenfd, (struct sockaddr*)&addr2, &len);
 
    INFOLOG("%s worker socket start and bound name = %s",proxy->proto->proto_name, addr2.sun_path);
 	
    return listenfd;	

    return 0;
}


int proxy_send_event(struct fctl_proxy_t* proxy,struct event_header_t* header){
/*    int     ret = 0, sockfd;
    struct sockaddr_un cliaddr, servaddr;

    sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd < 0){
        ERRLOG("socket error:%s", strerror(errno));
        return -3;
    }

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, proxy->worker_sock_path);
    
    int servlen = sizeof(struct sockaddr_un);
    for(;;){
        int n = sendto(sockfd, header, header->event_len, 0, 
            (struct sockaddr*)&servaddr, servlen);
        if (n < 0){
            if (errno == EINTR) continue;
            ERRLOG("send error:%s", strerror(errno));   
            ret = -1;
            break;    
        }
        if (n < header->event_len){
            ERRLOG("send error:%s", "send less data");
            ret = -2;
        }
        break;
    }
    
    return ret;
*/
    return 0;
}

struct fctl_proxy_t* proxy_new(struct fctl_proto_t* proto, int port, 
    int max_bind_port ,int min_bind_port){
    
    struct fctl_proxy_t* proxy = NULL;
	
    proxy = (struct fctl_proxy_t*)malloc(sizeof(struct fctl_proxy_t));
    if (proxy == NULL)
        return NULL;

    memset(proxy , 0  , sizeof(struct fctl_proxy_t));	

    proxy->max_bind_port = max_bind_port;
    proxy->min_bind_port = min_bind_port;
    
    proxy->port = port; 
    proxy->valid_port = max_bind_port;
    proxy->proto = proto;	

    INIT_LIST_HEAD(&proxy->session_head);
	
    proxy->get_port = proxy_get_valid_port;
    proxy->read = proxy_read;
    proxy->write = proxy_write;	
    proxy->listen = proxy_listen;
    proxy->worker_listen = proxy_worker_listen;
    proxy->send_event = proxy_send_event;
        	
    return proxy;	
}



