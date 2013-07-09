#ifndef __CACHE_H_
#define __CACHE_H_

#include "list.h"

struct common_cache_t{
    struct list_head list;
    int (*read)(struct common_cache_t* ,char* , int);
    void (*destroy)(struct common_cache_t*);
};

struct mem_cache_t{
    struct common_cache_t cache;
    int mem_len;
    int read_count;
    char mem[0];
};
struct mem_cache_t* mem_cache_new(char* buf , int buflen);

struct file_cache_t{
    struct common_cache_t cache;
    int fd;    
    char file_name[512];
};
struct file_cache_t* file_cache_new(char* filename );

struct cache_pool_t{
    int cache_count;
    struct list_head cache_head;
   
};
void cp_init(struct cache_pool_t* cp);
void cp_add_cache(struct cache_pool_t* cp , struct common_cache_t* cache);
int cp_read(struct cache_pool_t* cp ,char* buf, int buflen);
void cp_destroy(struct cache_pool_t* cp);


#endif
