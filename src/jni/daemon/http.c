#include "fctl.h"
#include "http_parser.h"

typedef enum
{
	HTTP_INITIALIZED = 0,
	HTTP_HEADER_RECV,
	HTTP_RESPONSE_HEADER_RECV,

} HTTP_STATUS;

struct httppriv_t{
    //http_parse_context hpc;
    char* cur_http_header;
    HTTP_STATUS remote_status;
};

extern  struct fctl_conf_t g_conf;

#ifdef HTTP_RECV_ALL_TRAFFIC
static inline int _is_http_traffic(struct fctl_session_t* session)
{
    if (session == NULL)
        return 0;

    if (session->remote_dst_port == 80)
        return 1;

    return 0;
}
#endif

/*
    http_connect can be called when a new http session is established.
*/
static int http_connect(struct fctl_session_t* session)
{    
    return 0;
}

/*
    When a http connection ends , this function would be called.   

*/
static int http_disconnect(struct fctl_session_t* session)
{
    return 0;
}

/*
   When th proxy receives some datas from http client , this function would be called.

*/
static int http_local_rev(struct fctl_session_t* session, char* buf , int buflen){
   
    int space = 0;
  
#ifdef HTTP_RECV_ALL_TRAFFIC
    if (!_is_http_traffic(session)){
        space = rwbuff_get_readn(&session->local_buf);
        rwbuff_read(&session->local_buf, space);
        return PROTO_SEND;
    }
#endif

    if (session->proto_status != HTTP_INITIALIZED)
        goto ret;
    
    
    // Search the http header end. 
    
    char * header_tail = strstr(buf , "\r\n\r\n");
    if ( header_tail == NULL)
        return PROTO_RESERVED;
    
    session->proto_status = HTTP_HEADER_RECV;
  
#if 0
    //save header for test
    FILE* fp = fopen("/tmp/header", "a+");
    if (fp) {
        fwrite(buf , 1, strlen(buf), fp);
        fclose(fp);
    }
#endif
    
  	int header_len = header_tail - buf + 4;
	int left_len = buflen - header_len;
	char* left_content = NULL;
	if (left_len > 0){
		char* left_content = (char*)malloc(left_len);
		memcpy(left_content, buf + header_len, left_len);	
	}
	struct http_parser_t hp = {0};
	http_update_req(&hp, buf, header_len);
	
	int j = 0;
	for (; j<header_len; j++){
		printf("%c", buf[j]);
	}
	
	char* new_url = strstr(hp.hlines[0].new_line, " ");
	if (new_url)
		new_url++;
	printf("url: %s\n", new_url); 	
	int need_proxy = url_need_proxy(new_url);
       
	if (session->remote_fd <= 0){
	 	int r = 0; 
	 	if (need_proxy){
    		char proxy_ip[32] = {0};
    		new_sogou_proxy_addr(proxy_ip, sizeof(proxy_ip));
    		printf(">>>proxy ip: %s\n", proxy_ip);
    		r = session->proxy->remote_connect(session->proxy, session, proxy_ip[0] == 0?NULL:proxy_ip, 80);
       }else
       	 	r = session->proxy->remote_connect(session->proxy, session, NULL, 0);
    
    	if (r < 0){
    		if (left_content)
    			free(left_content);
    		http_parser_reset(&hp);
            return r;
        }
    }
    
    if (need_proxy){
		rwbuff_rewind_writep(&session->local_buf);
	
 		int i = 0;
		for (; i < hp.cur_hline; i++) {
			char* line = NULL;
			int line_len = 0;
			if (hp.hlines[i].new_line != NULL){
				line = hp.hlines[i].new_line;
				line_len = hp.hlines[i].new_line_len;
				DBGLOG("n %s", hp.hlines[i].new_line );
			}else{
				line =  hp.hlines[i].ori_line;
				line_len =  hp.hlines[i].ori_line_len;
				DBGLOG("o %s", hp.hlines[i].ori_line);
			}
			if (line){
				rwbuff_append_data(&session->local_buf, line, line_len);
			}
		}
		rwbuff_append_data(&session->local_buf, "\r\n", 2);
		
		if (left_content){
			rwbuff_append_data(&session->local_buf, left_content, left_len);
		}
	}
	
	http_parser_reset(&hp);
	if (left_content)
		free(left_content);
    
 ret:        
    space = rwbuff_get_readn(&session->local_buf);
    rwbuff_read(&session->local_buf, space);
            
    return PROTO_SEND;
    
}

/*
   When th proxy receives some datas from http server , this function would be called.

*/
static int http_remote_rev(struct fctl_session_t* session, char* buf , int buflen)
{

#ifdef HTTP_RECV_ALL_TRAFFIC
    if (!_is_http_traffic(session)){
        int space = rwbuff_get_readn(&session->remote_buf);
        rwbuff_read(&session->remote_buf, space);
        return PROTO_SEND;
    }
#endif
	
	session->proto_status = HTTP_INITIALIZED;
 
    int space = rwbuff_get_readn(&session->remote_buf);
    rwbuff_read(&session->remote_buf, space);
    return PROTO_SEND;	

}


struct fctl_proto_t * http_new(void){
    struct fctl_proto_t* ret = NULL;
	
    ret = (struct fctl_proto_t*)malloc(sizeof(struct fctl_proto_t));
    if (ret == NULL)
        return NULL;
    memset(ret , 0  , sizeof(struct fctl_proto_t));	

    strcpy(ret->proto_name, "HTTP");
    ret->port = 80;	
    ret->on_connect= http_connect;
    ret->on_disconnect= http_disconnect;
    ret->on_local_rev= http_local_rev;
    ret->on_remote_rev= http_remote_rev;	

    return ret;
}

