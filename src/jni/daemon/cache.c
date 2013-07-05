#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "cache.h"

#define SAFE_CLOSE(x)	if (x > 0)	{while (close(x) == -1 && errno == EINTR);}
    

void cp_init(struct cache_pool_t* cp){
    cp->cache_count = 0;
    INIT_LIST_HEAD(&cp->cache_head);
}

void cp_add_cache(struct cache_pool_t* cp , struct common_cache_t* cache){
    list_add_tail(&cache->list, &cp->cache_head);
    cp->cache_count++;
   
}

int cp_is_empty(struct cache_pool_t* cp){
    
    return (cp->cache_count > 0)? 0 : 1;

}

int cp_read(struct cache_pool_t* cp ,char* buf, int buflen){
    
    for(;;){
        
        if (cp->cache_count == 0) 
            return 0;
   
        struct common_cache_t* cur_cache = 
            (struct common_cache_t*)cp->cache_head.next;    
        
        if ((unsigned long)cur_cache == (unsigned long)(&cp->cache_head))
            return 0;
           
        int n = cur_cache->read(cur_cache , buf , buflen);
        if (n <= 0){
            list_del(&cur_cache->list);
            cur_cache->destroy(cur_cache);
            cp->cache_count--;
        }else
            return n;
    
    }
    
    return 0;    
}

void cp_destroy(struct cache_pool_t* cp){
    struct list_head *pos =  NULL;
    struct common_cache_t* cur_cache = NULL;
    	  
    if (cp == NULL || cp->cache_count == 0) return;
        	  
    list_for_each(pos, &cp->cache_head){
        cur_cache = (struct common_cache_t*)pos;
        pos = pos->prev;
        list_del(&cur_cache->list);
        cur_cache->destroy(cur_cache);
    }
    cp->cache_count = 0;
}

static int mem_read(struct common_cache_t* cache,char* buf, int buflen){
    int real_count = 0;
    struct mem_cache_t* mcache = (struct mem_cache_t*)cache;
    
    
    int left = mcache->mem_len - mcache->read_count;
    if (left == 0) return 0;
        
    real_count = left > buflen?buflen: left;
    memcpy(buf , mcache->mem + mcache->read_count, real_count);
    
    mcache->read_count += real_count;
    
    return real_count;    
           
}

static void mem_destroy(struct common_cache_t* cache){
    if (cache != NULL)
        free(cache);
}


static inline void cache_init(struct common_cache_t* cache){
    INIT_LIST_HEAD(&cache->list);
}


static int file_read(struct common_cache_t* cache,char* buf, int buflen){
    
    if (cache == NULL) return 0;
    
    struct file_cache_t* fcache = (struct file_cache_t*)cache;
        
    if (fcache->fd < 0) return 0;
 
    int n = 0;
    do {   
        n = read(fcache->fd, buf , buflen);
    } while ( n < 0 && errno == EINTR);
                
    return n;    
           
}


static void file_destroy(struct common_cache_t* cache){
    
    if (cache == NULL) return ;
        
    
    struct file_cache_t* fcache = (struct file_cache_t*)cache;
    
    SAFE_CLOSE(fcache->fd);
    fcache->fd = -1;
    
    unlink(fcache->file_name);
        
    free(cache);
}

 
struct mem_cache_t* mem_cache_new(char* buf , int buflen){
    struct mem_cache_t* mcache = NULL;
    
    int cache_size = sizeof(struct mem_cache_t) + buflen +10;
    mcache = malloc(cache_size);
    if (mcache == NULL)
        return NULL;
    
    memset(mcache , 0 , cache_size);
        
    cache_init((struct common_cache_t*)mcache);
    
    mcache->cache.read = mem_read;
    mcache->cache.destroy = mem_destroy;
    
    memcpy(mcache->mem , buf , buflen);
    mcache->mem_len = buflen;
    
    return mcache;
}

struct file_cache_t* file_cache_new(char* filename ){
    struct file_cache_t* fcache = NULL;
    
    int cache_size = sizeof(struct file_cache_t);
    fcache = malloc(cache_size);
    if (fcache == NULL)
        return NULL;
    
    memset(fcache , 0 , cache_size);
        
    cache_init((struct common_cache_t*)fcache);
    
    fcache->cache.read = file_read;
    fcache->cache.destroy = file_destroy;
    
    strncpy(fcache->file_name, filename , sizeof(fcache->file_name)-1);
    fcache->fd = open(fcache->file_name, O_RDONLY);
    if (fcache->fd < 0) return NULL;
        
    return fcache;
}
