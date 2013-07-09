/*
 *  http_parser.h
 *  tcplognke
 *
 *  Created by Dazhi Li on 11-10-21.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include <netinet/in.h>

#define PR_NOT_HTTP	-1
#define PR_DONE		0
#define PR_PENDING	1

#define REQ_COMMON_FIELDS \
int reqid; \
int uid; \
unsigned int pid; \
int net_protocol;\
union {\
struct sockaddr_in	addr4;\
struct sockaddr_in6	addr6;\
} local;\
union {\
	struct sockaddr_in	addr4;\
	struct sockaddr_in6	addr6;\
} remote;\
char host[64];\
int urllen;\

struct http_nke_filter_req_t
{
	REQ_COMMON_FIELDS
};

struct header_line_t
{
	int type;
	char* ori_line;
	int ori_line_len;
	char* new_line;
	int new_line_len;
};

#define WF_MAX_HTTP_HEADER_LINE	32

#define WF_HTTP_HEADER_LINE_UNKKNOWN	0
#define WF_HTTP_HEADER_LINE_URL			1
#define WF_HTTP_HEADER_LINE_HOST		2
#define WF_HTTP_HEADER_LINE_COOKIE		3

struct http_parser_t
{
	/*int reqid;
	int net_protocol;
	union {
		struct sockaddr_in	addr4;		// ipv4 remote addr
		struct sockaddr_in6	addr6;		// ipv6 remote addr 
	} remote;
	char url[1024];
	char host[256];*/
	
	REQ_COMMON_FIELDS
	char url[1024];	
	char cache_line[2048];
	int cache_line_index;
	short status;
	char http_version_str[16];
    char method[16];
	
	int safe_search_enabled;
	struct header_line_t hlines[WF_MAX_HTTP_HEADER_LINE];
	int cur_hline;
	int hostlen;
};

int http_update_req(struct http_parser_t* hp, char* buffer, int buflen1);
int http_get_new_request_mbuf(struct http_parser_t* hp, void *data);
int http_parser_reset(struct http_parser_t* hp);
int new_sogou_proxy_addr(char* ip, int maxip);
int url_need_proxy(const char* url);

#endif
