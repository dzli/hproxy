#include "fctl.h"

static int session_on_local_recv(struct fctl_session_t* session){

    if (session == NULL || session->proxy == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
    
    //The connection to real server is not be established.
    //It is not an error, just return.        
//    if (session->remote_status == SESSION_CONNECTING)
//        return 0;
    
    char * local_buf = rwbuff_get_writep(&session->local_buf);
    int space = rwbuff_get_writen(&session->local_buf);

    
    DBGLOG("%d space left in local buffer.", space);
       
    if ( space == 0) {
        return 0;
    }
 
    int n;
 again:   
    n = session->proxy->read(session->local_fd, local_buf , space);
    
    if (n < 0) {
        
        if ( errno == EINTR) goto again;
        //  none block io is used, so EWOULDBLOCK is ignored.        
        if ( errno != EWOULDBLOCK) {
            DBGLOG("read error:%s",strerror(errno));
            return  -2;
        }else
            return 0;

    }else if ( n == 0){
         
        // FIN received from local, but mybe there are data in local_buff 
        // which need to be sent, So local_status is SESSION_CLOSING 
        //instead of SESSION_CLOSED             
        session->local_status = SESSION_CLOSING;
        session->closing_time = time(0);
        DBGLOG("FIN is received from local.");
        
        return 0;
    }

    DBGLOG("%d data are received from local.", n);

     session->closing_time = time(0);

    //rejust the write pointer in local_buf
    rwbuff_write(&session->local_buf , n);

    local_buf = rwbuff_get_readp(&session->local_buf);
    space = rwbuff_get_readn(&session->local_buf);

    DBGLOG("there are %d data to be handled by protocal handler.", space);
    local_buf[space] = '\0';
    
    // call the protocol handler 
    int action = session->proxy->proto->on_local_rev(session , local_buf, space);
    
    if ( action < 0 )
        return -1;
          
    return 0; 
}


static void session_check_local_extra_data(struct fctl_session_t* session){
   
    if (rwbuff_get_sendn(&session->local_buf) != 0 
        || rwbuff_get_readn(&session->local_buf)!= 0) return;
    if (cp_is_empty(&session->local_buf.cache_pool) ) return;
    
    
    char * buf = rwbuff_get_writep(&session->local_buf);
    int space = rwbuff_get_writen(&session->local_buf);
    
    if ( space == 0) {
        return ;
    }
    int n = cp_read(&session->local_buf.cache_pool, buf , space);
    
    if (n > 0){
        rwbuff_write(&session->local_buf , n);
        rwbuff_read(&session->local_buf, n); 
    }
       
        
}

static int session_on_local_send(struct fctl_session_t* session){
    
     if (session == NULL || session->proxy == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
    
    if (session->remote_status != SESSION_CONNECTED)
        return 0;

    session_check_local_extra_data(session);
    
    char* local_buf = rwbuff_get_sendp(&session->local_buf);
    int space = rwbuff_get_sendn(&session->local_buf);

    if ( space == 0){
        DBGLOG("no data in local buffer need to be sent");
        goto ret;
    }

    DBGLOG("%d data in local buffer need to be sent!", space);

    int n;
again:    
    n = session->proxy->write(session->remote_fd, local_buf , space);
        
    if (n <= 0) {
        
        if ( errno == EINTR) goto again;
            
        if ( errno != EWOULDBLOCK ){ 
            ERRLOG("write error:%s",strerror(errno));
            return -2;
        }
        
        //ERRLOG("%s", strerror(errno));
        
        /*session->local_retry_times++;
        if ( session->local_retry_times >= 10)
            return -2;
        */
        return 1;
        
    }

    session->closing_time = time(0);

    DBGLOG(" %d data in local buffer are sent successfully.", n);
    
    //rejust the sent pointer in local_buf
    rwbuff_send(&session->local_buf, n);
    
    session_check_local_extra_data(session);
    
ret:
    if ( rwbuff_get_sendn(&session->local_buf) == 0  
        && session->local_status == SESSION_CLOSING) {
        
        session->local_status = SESSION_CLOSED;
    
    }
    
    return 0;
}

static int session_on_remote_recv(struct fctl_session_t* session){
    
    if (session == NULL || session->proxy == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
    
    char * buf = rwbuff_get_writep(&session->remote_buf);
    int space = rwbuff_get_writen(&session->remote_buf);
    
    DBGLOG("%d space left in remote buffuser.", space);
     
    if ( space == 0) {
        return 0;
    }

    int n;
again:    
    n = session->proxy->read(session->remote_fd, buf , space);
   
    if (n < 0) {
        
        if ( errno == EINTR) goto again;
            
        if ( errno != EWOULDBLOCK){ 
            DBGLOG("remote read error:%s",strerror(errno));
            return  -2;
        }
        
        return 0;

    }else if ( n == 0){
        
        session->remote_status = SESSION_CLOSING;
        session->closing_time = time(0);
        session->proxy->proto->on_remote_rev(session , NULL, 0);
        DBGLOG("FIN is received from remote.");
        
        return 0;
    }
    
    DBGLOG("%d data are received from remote.", n);

    session->closing_time = time(0);

    rwbuff_write(&session->remote_buf , n);

    buf = rwbuff_get_readp(&session->remote_buf);
    space = rwbuff_get_readn(&session->remote_buf);

    
    int action = session->proxy->proto->on_remote_rev(session , buf, space);
   
    if (action < 0)
        return -3;
    // rwbuff_read(&session->remote_buf, space);

    return 0; 
}

static void session_check_remote_extra_data(struct fctl_session_t* session){
   
    if (rwbuff_get_sendn(&session->remote_buf) != 0 
        || rwbuff_get_readn(&session->remote_buf)!= 0) return;
    if (cp_is_empty(&session->remote_buf.cache_pool) ) return;
    
    
    char * buf = rwbuff_get_writep(&session->remote_buf);
    int space = rwbuff_get_writen(&session->remote_buf);
    
    if ( space == 0) {
        return ;
    }
    int n = cp_read(&session->remote_buf.cache_pool, buf , space);
    
    if (n > 0){
        rwbuff_write(&session->remote_buf , n);
        rwbuff_read(&session->remote_buf, n); 
    }
       
        
}

static int session_on_remote_send(struct fctl_session_t* session){
    
    if (session == NULL || session->proxy == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
    
    session_check_remote_extra_data(session);

    char* buf = rwbuff_get_sendp(&session->remote_buf);
    int space = rwbuff_get_sendn(&session->remote_buf);

    DBGLOG("%d data in remote buffer need to be sent!", space);

    if ( space == 0){
        DBGLOG("no data in remote buffer need to be sent");
        goto ret;
    }

    int n;
again:
    n = session->proxy->write(session->local_fd, buf , space);
    
    if (n <= 0) {
        
        if ( errno == EINTR) goto again;
              
        if ( errno != EWOULDBLOCK){ 
            ERRLOG("write error:%s",strerror(errno));
            return -2;
        }
        
        //ERRLOG("%s", strerror(errno));
        
       /* session->remote_retry_times++;
        if ( session->remote_retry_times >= 10)
            return -2;
        */
        return 1;
        
    }
    
    DBGLOG(" %d data in remote buffer are sent successfully.", n);
   // buf[space] = 0;
     session->closing_time = time(0);   

    rwbuff_send(&session->remote_buf, n);

    session_check_remote_extra_data(session);

ret :
     if ( rwbuff_get_sendn(&session->remote_buf) == 0  
        && session->remote_status == SESSION_CLOSING) {
       
        session->remote_status = SESSION_CLOSED;
    
    }
    
    return 0;
}

static int session_free(struct fctl_session_t* session){
    
     if (session == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
    
    free(session);
}

static int session_close(struct fctl_session_t* session){
     
    if (session == NULL){
        ERRLOG("session can not be NULL!");
        return -1;
    }
        
     SAFE_CLOSE(session->remote_fd) ;
     SAFE_CLOSE(session->local_fd) ;
     
     session->remote_fd = -1;
     session->local_fd = -1;
     
     cp_destroy(&session->remote_buf.cache_pool);
     cp_destroy(&session->local_buf.cache_pool);        
        
     session->local_status = SESSION_CLOSED;
     session->remote_status = SESSION_CLOSED;
    
    return 0;
}

static int session_is_closed(struct fctl_session_t* session){
    
    if (session == NULL){
        ERRLOG("session can not be NULL!");
        return 0;
    }
    
    if (session->local_status == SESSION_CLOSED
        && session->remote_status == SESSION_CLOSED){

        return 1;
    }
    
      
    if ((session->local_status == SESSION_CLOSED
           || session->remote_status == SESSION_CLOSED)
        && ( (time(0) - session->closing_time) >= SESSION_TIMEOUT)
       )
       return 1;
       
    
    if (   (session->local_status == SESSION_CLOSING
           || session->remote_status == SESSION_CLOSING)
        && ( (time(0) - session->closing_time) >= SESSION_TIMEOUT)
       )
       return 1;
   
    
    return 0;
}


struct fctl_session_t* session_new(struct fctl_proxy_t* proxy , int fd, struct sockaddr_in *local_addr){
    struct fctl_session_t* ret = NULL;
	
	if (proxy == NULL){
        ERRLOG("proxy can not be NULL!");
        return NULL;
    }
    
    if (local_addr == NULL){
        ERRLOG("local_addr can not be NULL!");
        return NULL;
    }
    
    if (fd < 0){
        ERRLOG("fd can not less than 0!");
        return NULL;
    }
    
    ret = (struct fctl_session_t*)malloc(sizeof(struct fctl_session_t));
   
    if (ret == NULL){
        ERRLOG("malloc error!")
        return NULL;
    }

    memset(ret , 0  , sizeof(struct fctl_session_t));	
    
    ret->proxy= proxy;
    ret->local_fd = fd;
    ret->local_addr = *local_addr;
	
    ret->free = session_free;  
    ret->on_local_recv = session_on_local_recv;
    ret->on_remote_recv = session_on_remote_recv;
    ret->on_local_send = session_on_local_send;
    ret->on_remote_send = session_on_remote_send;
    ret->close = session_close;
    ret->is_closed = session_is_closed;

    INIT_LIST_HEAD(&ret->list);

    rwbuff_init(&ret->local_buf);
    rwbuff_init(&ret->remote_buf);
      
    if (proxy->cur_session_id >= 65535) 
        proxy->cur_session_id = 1;
    else
        proxy->cur_session_id++; 
           
	ret->session_id = proxy->cur_session_id;
	ret->spool.fd = -1;

    ret->create_time = time(0);	

    return ret;
}
