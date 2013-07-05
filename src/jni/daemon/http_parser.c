/*
 *  http_parser.c
 *  tcplognke
 *
 *  Created by Dazhi Li on 11-10-21.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

//#define DEBUG_PARSER	1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <inttypes.h>
#include <time.h>
#include <arpa/inet.h>
#include "http_parser.h"

#define PARSE_STA_INIT			0
#define PARSE_STA_SEEN_GET		1
#define PARSE_STA_GOT_URL		2
#define PARSE_STA_GOT_HOST		3
#define PARSE_STA_NOT_HTTP		99


struct matched_url 
{
	char* pattern;
	int len;
	int match_type;
};


static struct matched_url proxy_url_prefixes[] =
{
	{"http://v.youku.com/player/", 0,0},
    {"http://api.youku.com/player/",0,0},
    {"http://v2.tudou.com/",0,0},
    {"http://www.tudou.com/a/",0,0},
    {"http://www.tudou.com/v/",0,0},
    {"http://s.plcloud.music.qq.com/fcgi-bin/p.fcg",0,0},
    { "http://hot.vrs.sohu.com/",0,0},
    {"http://live.tv.sohu.com/live/player",0,0},
    {"http://hot.vrs.letv.com/",0,0},
    {"http://g3.letv.cn/",0,0},
    {"http://data.video.qiyi.com/",0,0},

    { "http://220.181.61.229/",0,0},
    {"http://61.135.183.45/",0,0},
    {"http://61.135.183.46/",0,0},
    {"http://220.181.19.218/",0,0},
    {"http://220.181.61.213/",0,0},
    {"http://220.181.118.181/",0,0},
    {"http://123.126.48.47/",0,0},
    {"http://123.126.48.48/",0,0},

    {"http://vv.video.qq.com/",0,0},
    {"http://geo.js.kankan.xunlei.com/",0,0},
    {"http://web-play.pptv.com/",0,0},
    {"http://web-play.pplive.cn/",0,0},
    {"http://dyn.ugc.pps.tv/",0,0},
    {"http://inner.kandian.com/",0,0},
    {"http://ipservice.163.com/",0,0},
    {"http://zb.s.qq.com/",0,0},
    {"http://ip.kankan.xunlei.com/",0,0},

    {"http://music.sina.com.cn/yueku/intro/",0,0},
    {"http://music.sina.com.cn/radio/port/webFeatureRadioLimitList.php",0,0},
    {"http://play.baidu.com/data/music/songlink",0,0},

    {"http://v.iask.com/v_play.php",0,0},
    {"http://v.iask.com/v_play_ipad.cx.php",0,0},
    {"http://tv.weibo.com/player/",0,0},

    {"http://www.yinyuetai.com/insite/",0,0},
    {"http://www.yinyuetai.com/main/get-video-info",0,0},
    {"http://vdn.apps.cntv.cn/api/getHttpVideoInfo.do",0,0},
    
    {"dpool.sina.com.cn/iplookup",0,1},
    {"/vrs_flash.action",0,1},
    {"/?prot=2&type=1",0,1},
    {"/?prot=2&file=/",0,1},
    
    //comes from server extra list 
    {"http://api.3g.youku.com/layout",0,0},
    {"http://api.tv.sohu.com/",0,0},
    {"http://access.tv.sohu.com/",0,0},
    {"http://3g.music.qq.com/",0,0},
    {"http://mqqplayer.3g.qq.com/",0,0},
    {"http://proxy.music.qq.com/",0,0},
    {"http://api.3g.tudou.com/",0,0},
    {"http://mobi.kuwo.cn/",0,0},
    {"http://mobilefeedback.kugou.com/",0,0},
    {"http://api.3g.youku.com/v3/play/address",0,0},
    {"http://api.3g.youku.com/openapi-wireless/videos/",0,0}, //need pcre
    {"http://api.3g.youku.com/videos/",0,0}, // need pcre
    {"http://play.api.3g.tudou.com/v3_1/",0,0},
    {"http://iface2.iqiyi.com/php/xyz/iface/",0,0},

    // for 3rd party's DNS for Apple TV (see pull request #78)
    //'http://180.153.225.136/*',
    //'http://118.244.244.124/*',
    //'http://210.129.145.150/*',
   
};

int url_need_proxy(const char* url)
{
	if (url == NULL || *url == 0)
		return 0;
		
	int i = 0;
	for(; i < sizeof(proxy_url_prefixes)/sizeof(proxy_url_prefixes[0]); i++){
		struct matched_url* mu = &proxy_url_prefixes[i];
		if (mu->match_type == 0){
			if (mu->len == 0)
				mu->len = strlen(mu->pattern);
			if (strncasecmp(url, mu->pattern, mu->len) == 0)
				return 1;
		}else if (mu->match_type == 1){
			if (strstr(url, mu->pattern) != NULL)
				return 1;
		}
	}
	return 0;
}

static int new_random_ip(char* ip, int maxip) 
{
    if (ip == NULL)
        return -1;
    int random_num = (random()/(double)RAND_MAX) * 254 + 1; 
    snprintf(ip, maxip, "220.181.111.%d", random_num); // 1 ~ 254
    return 0;
}

int new_sogou_proxy_addr(char* ip, int maxip) {
    char new_addr[256] = {0};
    int maxaddr = sizeof(new_addr);
    static char resolved_addr[64][32] = {{0}}; 

#if 0 //for test use
    strncpy(ip, "220.181.118.155", maxip-1);
    return 0;
#endif
    char*  other_ip_addrs[] = {
        "220.181.118.128",
    };

    int random_num = (random()/(double)RAND_MAX) * (16 + 16 + sizeof(other_ip_addrs)/sizeof(other_ip_addrs[0]));  // 0 ~ 15 edu, 0 ~ 15 dxt
    if (random_num < 16) {
        if (8 == random_num || 12 == random_num) {
            return  new_sogou_proxy_addr(new_addr, maxaddr); // just retry
        }else
            snprintf(new_addr, maxaddr, "h%d.dxt.bj.ie.sogou.com", random_num);
    } else if (random_num < 16 + 16) {
        snprintf(new_addr, maxaddr, "h%d.edu.bj.ie.sogou.com", random_num - 16);
    } else {
        snprintf(new_addr, maxaddr, "%s", other_ip_addrs[random_num - 16 - 16]);
    }

    //resolve the hostname, then put it into cache
    if (resolved_addr[random_num][0] == 0){
        struct addrinfo hints, *res = NULL, *res0 = NULL;
        int error;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        error = getaddrinfo(new_addr, "80", &hints, &res0);
        if (error || res0 == NULL) {
            return -2;
        }
        res = res0;
        char o[32] = {0};
        inet_ntop(res->ai_family, &((struct  sockaddr_in*)res->ai_addr)->sin_addr,o, sizeof(o));
        freeaddrinfo(res0);
        strncpy(resolved_addr[random_num], o, sizeof(resolved_addr[random_num])-1);
    }
    strncpy(ip, resolved_addr[random_num], maxip-1);

    return 0;
}

static int new_sogou_auth_str(char* auth, int maxauth) 
{
    char auth_str[] = "/30/853edc6d49ba4e27";
    int i = 0, j = 0;
    char tmp_str[5];

    for (; i < 8; i++) {
        snprintf(tmp_str, sizeof(tmp_str), "%04lX", random() * 65536);
        j += snprintf(auth+j, maxauth - j, "%s", tmp_str);
    }
    snprintf(auth+j, maxauth-j, "%s", auth_str);
    return 0;
}


static int  compute_sogou_tag(const char* timestamp, const char* hostname, char* tag, int maxtag) 
{
    char s[256] = {0};
    
    snprintf(s, sizeof(s), "%s%s%s", timestamp, hostname, "SogouExplorerProxy");

    int total_len = strlen(s);
    int numb_iter = total_len / 4;
    int numb_left = total_len % 4;
    int64_t hash = total_len;  // output hash tag

    int i = 0;
    for (i = 0; i < numb_iter; i++) {
        unsigned int vv =*( (int*)(s+4*i));
        unsigned int  a = (vv & 0xFFFF);
        unsigned int  b = (vv >> 16);
        hash += a;
        hash = hash ^ (((hash<<5)^b) << 0xb);
        // To avoid overflows
        hash &= 0xffffffff;
        hash += hash >> 0xb;
    }

    switch (numb_left) {
        case 3:
            hash += (s[total_len - 2] << 8) + s[total_len - 3];
            hash = hash ^ ((hash ^ s[total_len-1]*4) << 0x10);
            hash &= 0xffffffff;
            hash += hash >> 0xb;
            break;
        case 2:
            hash += (s[total_len - 1] << 8) + s[total_len - 2];
            hash ^= hash << 0xb;
            hash &= 0xffffffff;
            hash += hash >> 0x11;
            break;
        case 1:
            hash += s[total_len - 1];
            hash ^= hash << 0xa; 
            hash &= 0xffffffff;
            hash += hash >> 0x1; 
            break;
        default:
            break;
    }

    hash ^= hash << 3;
    hash &= 0xffffffff;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash = hash & 0xffffffff;
    hash += hash >> 0x11;
    hash ^= hash << 0x19;
    hash = hash & 0xffffffff;
    hash += hash >> 6;
    hash = hash & 0xffffffff;

    snprintf(tag, maxtag, "%08" PRIx64, hash);
    return 0;
}


static void* wfmalloc(uint32_t size)
{
	return malloc(size);
}

static void wffree(void* buf, uint32_t size)
{
	if (buf != NULL){
		free(buf);
	}
}

static int cache_buf(struct http_parser_t* hp, char* buffer, int buflen)
{
    if ((sizeof(hp->cache_line) - hp->cache_line_index -1) < buflen)
        return -1; 
	
    if (buflen > 0){ 
        memcpy(hp->cache_line + hp->cache_line_index, buffer, buflen);
        hp->cache_line_index += buflen;
    }   
    return 0;
}

static char* strstrn(char* buf , int buflen , char*str){
    int i = 0;
    int str_len = strlen(str);
    int maxi = buflen - str_len ;
    while (i <= maxi && strncasecmp(buf+i, str, str_len) ) 
        i++;
    if (i > maxi) 
        return NULL;
    return buf+i;
}

static int extract_url(struct http_parser_t* hp, char* buffer, int buflen)
{
    char* start = strstrn(buffer,buflen, " "); //skip "GET ";
    char* hstart = strstrn(buffer, buflen, "HTTP/");
    if (hstart == NULL || start == NULL)
        return -1;
	
    if (start-buffer >= sizeof(hp->method))
        return -2;
    memset(hp->method, 0, sizeof(hp->method));
    memcpy(hp->method, buffer, start-buffer);
    
    start++;

	int httpvlen = buflen - (hstart - buffer);
	if (httpvlen > sizeof(hp->http_version_str) - 1)
		httpvlen = sizeof(hp->http_version_str) - 1;
	memcpy(hp->http_version_str, hstart, httpvlen);
	hp->http_version_str[httpvlen] = 0;
	
    int urllen = (hstart - start - 1);
    if (urllen > (sizeof(hp->url) - 1))
        urllen = sizeof(hp->url) - 1;
    strncpy(hp->url, start, urllen);
    hp->url[urllen] = 0;
	hp->urllen = urllen;
    return 0;
	
}
static int get_header_line(struct http_parser_t* hp, char* buffer, int* buflen, char** newbuf, int* newbuflen)
{
    int ret = 0;
	
    if (buffer[0] == '\n'){
        buffer++;
        *buflen = *buflen - 1;
    }
	
    char* end = strstrn(buffer, *buflen, "\r");
    if (end == NULL){
        ret = cache_buf(hp, buffer, *buflen);
        if (ret <0)
            return ret;
        ret = 1;
    }else{
        if (hp->cache_line_index == 0){
            *newbuf = buffer;
            *newbuflen = end - buffer;
            *buflen = *buflen - *newbuflen -1;
        }else{
            ret = cache_buf(hp, buffer, end - buffer);
            if (ret <0)
                return ret;
            *newbuf = hp->cache_line;
            *newbuflen = hp->cache_line_index;
          	*buflen = *buflen - (end - buffer + 1);
			hp->cache_line_index = 0;
        }
    }
    return ret;
}

static int gethost(struct http_parser_t* hp, char* buffer, int buflen)
{
	int i = 0;
	while(i < buflen && buffer[i] != ':')
		i++;
	if (i >= buflen)
		return -1;
	
	//4 is the lentgh of host
	if ((i != 4 ) || strncasecmp("Host", buffer, 4) != 0)
		return 1;
	
	//skip ": "
	i+=2;
	if (i < buflen){
		int len = (buflen-i) > (sizeof(hp->host) - 1)? (sizeof(hp->host) - 1): (buflen - i);
		memcpy(hp->host, buffer + i, len);  
		hp->host[len] = 0;
		hp->hostlen = len;
	}
	return 0;
}

#if 0
static  void printn(char* line , int len)
{
	int i = 0;
	for(; i < len ; i++){
		printf("%c", line[i]);
	}
	printf("\n");
}
#endif

static char* change_url_line(struct http_parser_t* hp, int* new_line_len)
{
    if (hp == NULL || hp->url[0] == 0 || hp->host[0] == 0)
        return NULL;
	
	char* newline = NULL;
	int hostlen = strlen(hp->host);
	int newline_len = 0;
	
    newline_len = hp->urllen + hostlen + 128; //128 to make it long enough 
    newline = (char*)malloc(newline_len);
    if (newline == NULL)
        return NULL;
    snprintf(newline, newline_len, "%s http://%s%s %s\r\n", hp->method, hp->host, hp->url, hp->http_version_str);

	*new_line_len = strlen(newline);
	return newline;
}

#if 0
static int isletter(char c){
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return 1;
	return 0;
}
#endif

static int set_new_header_line(struct http_parser_t* hp, int linetype, const char* newline, int newline_len)
{	
    int i = 0;
	for (; i < hp->cur_hline; i++) {
		if (hp->hlines[i].type ==  linetype){
			hp->hlines[i].new_line = (char*)newline;
			hp->hlines[i].new_line_len = newline_len;
			break;
		}
	}
	return 0;
}

static int handle_header_line(struct http_parser_t* hp, char* line, int linelen)
{
	int ret = 0;
	int linetype = WF_HTTP_HEADER_LINE_UNKKNOWN;
	char* newline = NULL;
	int newline_len = 0;
	
	if (line == NULL || linelen == 0)
		return ret;
	
	if (hp->url[0] == 0){
		linetype = WF_HTTP_HEADER_LINE_URL;
		if (extract_url(hp, line, linelen) < 0){
			ret =  -1;
		}
		//printf("parse URL : %s\n", hp->url);
	}
	
	if ((linelen >= 6 ) && strncasecmp("Host: ", line, 6) == 0){
		linetype = WF_HTTP_HEADER_LINE_HOST;
		gethost(hp, line, linelen);
        int tmpstr_len = 0;
        char* tmpstr = change_url_line(hp, &tmpstr_len);
        set_new_header_line(hp, WF_HTTP_HEADER_LINE_URL, tmpstr, tmpstr_len);
	}
	
    if ((linelen >= 12 ) && strncasecmp("User-Agent: ", line, 12) == 0){
        if (strstrn(line, linelen, "CloudFront") == NULL || strstrn(line, linelen, "CloudFlare") == NULL){
 #define NEW_AGENT "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_3) AppleWebKit/537.31 (KHTML, like Gecko) Chrome/26.0.1410.65 Safari/537.31\r\n"
            newline = strdup(NEW_AGENT);
            newline_len = strlen(NEW_AGENT);
        }
    }
	
	int ori_line_len = linelen + 2; //plus "\r\n";
	char* ori_line = wfmalloc(ori_line_len + 1);
	if (ori_line){
		memcpy(ori_line, line, linelen);
		memcpy(ori_line + linelen, "\r\n", 2);
		ori_line[ori_line_len] = 0;
		hp->hlines[hp->cur_hline].new_line = newline;
		hp->hlines[hp->cur_hline].new_line_len = newline_len;
		hp->hlines[hp->cur_hline].ori_line = ori_line;
		hp->hlines[hp->cur_hline].ori_line_len = ori_line_len;
		hp->hlines[hp->cur_hline].type = linetype;
		hp->cur_hline++;
		if (hp->cur_hline >= WF_MAX_HTTP_HEADER_LINE)
			return -1;
	}
	

	return ret;
}

static int add_new_header_fields(struct http_parser_t* hp)
{
    /*char sogo_tag[64] = {0};
    int i = 0;
    for(; i < 16; i++){
        new_sogou_proxy_addr(sogo_auth, sizeof(sogo_auth));
        printf("sogo addr: %s\n", sogo_auth);
    }
    return;*/

    char new_field[256] = {0};
    int l = 0;
    l = snprintf(new_field, sizeof(new_field), "%s", "X-Sogou-Auth: ");
    new_sogou_auth_str(new_field+l, sizeof(new_field)-l);
    handle_header_line(hp, new_field, strlen(new_field));

    time_t now = time(0);
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%lx", now);
    l = snprintf(new_field, sizeof(new_field), "X-Sogou-Timestamp: %s", timestamp);
    handle_header_line(hp, new_field, l);

    l = snprintf(new_field, sizeof(new_field), "%s", "X-Sogou-Tag: ");
    compute_sogou_tag(timestamp, hp->host, new_field+l, sizeof(new_field)-l);
    handle_header_line(hp, new_field, strlen(new_field));

    l = snprintf(new_field, sizeof(new_field), "%s", "X-Forwarded-For: ");
    new_random_ip(new_field+l, sizeof(new_field)-l);
    handle_header_line(hp, new_field, strlen(new_field));
    return 0;
}

int http_update_req(struct http_parser_t* hp, char* buffer, int buflen1)
{
    int ret = 0;

    if (hp == NULL)
        return -1;
	
    if (buffer == NULL || buflen1 < 0)
        return 0;
	if (hp->status == PARSE_STA_NOT_HTTP)
        return PR_NOT_HTTP;;
	
    int buflen = buflen1;
    int linelen;
    char* line;
    char* endbuf = buffer + buflen1;
	
    while (buflen > 0) {
        switch (hp->status) {
            case PARSE_STA_INIT:
			{
				//here , for performance , I assume that "GET " must be in one packet.
				if (buflen >= 4 && 
					(memcmp("GET ", buffer, 4) == 0 ||
					 memcmp("POST", buffer, 4) == 0 ||
					 memcmp("HEAD", buffer, 4) == 0 ||
					 memcmp("PUT ", buffer, 4) == 0)){
					hp->status = PARSE_STA_SEEN_GET;
				}else{
					hp->status = PARSE_STA_NOT_HTTP;
					return PR_NOT_HTTP;
				}
			}
                break;
            case PARSE_STA_SEEN_GET:
			{
				ret = get_header_line(hp, buffer, &buflen, &line, &linelen);
				if (ret < 0){
					hp->status = PARSE_STA_NOT_HTTP;
					return PR_NOT_HTTP;
				}
				if (ret > 0){
					ret = PR_PENDING;
					return ret;
				}
				if (linelen == 0){
                    add_new_header_fields(hp);
					return PR_DONE;
				}
				
				ret = handle_header_line(hp, line, linelen);
				
				if (ret != 0){
					hp->status = PARSE_STA_NOT_HTTP;
					return PR_NOT_HTTP;
				}
				
				buffer = endbuf - buflen;
			}
                break;
					
            default:
                break;
        }
		
    }

	return 0;
}

int http_parser_reset(struct http_parser_t* hp)
{
	if (hp == NULL)
		return -1;
    int i = 0;
	for (; i < hp->cur_hline; i++) {
		if (hp->hlines[i].new_line != NULL){
			wffree(hp->hlines[i].new_line, hp->hlines[i].new_line_len + 1);
			hp->hlines[i].new_line = NULL;
		}
		if (hp->hlines[i].ori_line != NULL){
			wffree(hp->hlines[i].ori_line, hp->hlines[i].ori_line_len + 1);
			hp->hlines[i].ori_line = NULL;
		}
	}
	memset(hp, 0, sizeof(*hp));
	return 0;
}

/*char* http_get_new_request(struct http_parser_t* hp, void *data)
{
	if (hp == NULL || data == NULL)
		return -1;
	
	mbuf_t* new_data = (mbuf_t*)data;
	int total_len = 0;
	
	for (int i = 0; i < hp->cur_hline; i++) {
		if (hp->hlines[i].new_line != NULL){
			mbuf_copyback(*new_data, total_len, hp->hlines[i].new_line_len, hp->hlines[i].new_line, MBUF_WAITOK);
			total_len+=  hp->hlines[i].new_line_len;
			//printf("%s", hp->hlines[i].new_line);
			
		}else{
			mbuf_copyback(*new_data, total_len, hp->hlines[i].ori_line_len, hp->hlines[i].ori_line, MBUF_WAITOK);
			total_len+= hp->hlines[i].ori_line_len;
			//printf("%s", hp->hlines[i].ori_line);
		}
	}

	//youtube education filter
	if (strstrn(hp->host, hp->hostlen, "youtube.com") !=0){
		lck_mtx_lock(gmutex);
		if (gYoutubeEduFilterLen > 0){
			//printf("!!! insert education filter %d: %s: %s\n",gYoutubeEduFilterLen, gYoutubeEduFilter,hp->url);
			mbuf_copyback(*new_data, total_len, gYoutubeEduFilterLen, gYoutubeEduFilter, MBUF_WAITOK);
			total_len += gYoutubeEduFilterLen;
			mbuf_copyback(*new_data, total_len, 2, "\r\n", MBUF_WAITOK);
			total_len += 2;
		}
		lck_mtx_unlock(gmutex);
	}
	mbuf_copyback(*new_data, total_len, 2, "\r\n", MBUF_WAITOK);
	//printf("%s", "\r\n");
	return 0;
}
*/

#ifdef TEST_PARSER
int main(int argc, char* argv[])
{
    char sogo_tag[64] = {0};
    char sogo_auth[128] = {0};
    int i = 0;
    for(; i < 16; i++){
        new_sogou_proxy_addr(sogo_auth, sizeof(sogo_auth));
        printf("sogo addr: %s\n", sogo_auth);
    }
   // return;

	printf("need: %d\n", url_need_proxy("http://hot.vrs.sohu.com/123"));
	printf("need: %d\n", url_need_proxy("http://hot.vrs.sohu.comddd/123"));
	printf("need: %d\n", url_need_proxy("http://123/dpool.sina.com.cn/iplookup"));
    //new_sogou_auth_str(sogo_auth, sizeof(sogo_auth));
    //printf("auth str: %s\n", sogo_auth);
    return;

   // char sogo_tag[64] = {0};
   // compute_sogou_tag("111111", "www.sohud.com", sogo_tag, sizeof(sogo_tag));
   // return 0;
    
	const char* filename = "5.txt";
	if (argc >= 2){
		filename = argv[1];
	}
	
	FILE* fp = fopen(filename, "r");
	if (fp == NULL){
		fprintf(stderr, "failed to open file %s\n", filename);
		return 2;
	}
	
	struct http_parser_t hp = {0};
	char buf[4096];
	int n = 0;
	
	while ((n = fread(buf,1, sizeof(buf), fp)) > 0) {
		http_update_req(&hp, buf, n);
	}
	
    //int i = 0;
	for (; i < hp.cur_hline; i++) {
		if (hp.hlines[i].new_line != NULL){
			printf("n %s", hp.hlines[i].new_line );
		}else{
			printf("o %s", hp.hlines[i].ori_line);
		}
	}
	fclose(fp);
	return 0;
}
#endif

