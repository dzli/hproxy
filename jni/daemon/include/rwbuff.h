#ifndef __RWBUFF_H_
#define __RWBUFF_H_

#include <sys/uio.h>
#include "cache.h"

#define RWBUFF_LEN 32*1024
#define RWBUFF_PRESERVE_LEN 1024

struct rwbuff_t{
    char buf[RWBUFF_LEN + RWBUFF_PRESERVE_LEN];
    char* readp;
    char* writep;
    char* sendp;
    int buf_len;
    int buf_default_len;

    /*int extra_data_usage;   //0: extra data does not exist. 1: data to be sent
    int extra_data_count;
    int extra_data_read_count;;
    struct iovec extra_data[10000];
    int extra_file;
    char extra_file_name[512];*/
    struct cache_pool_t cache_pool;
};

inline struct rwbuff_t * rwbuff_new();
inline int rwbuff_init(struct rwbuff_t * rwbuff);
inline int rwbuff_init_ptr(struct rwbuff_t * rwbuff);

inline int rwbuff_write(struct rwbuff_t *rwbuff, int n);
inline int rwbuff_get_writen(struct rwbuff_t *rwbuff);
inline char* rwbuff_get_writep(struct rwbuff_t *rwbuff);
inline int rwbuff_rewind_writep(struct rwbuff_t * rwbuff);

inline int rwbuff_read(struct rwbuff_t *rwbuff, int n);
inline int rwbuff_get_readn(struct rwbuff_t *rwbuff);
inline char* rwbuff_get_readp(struct rwbuff_t *rwbuff);

inline int rwbuff_send(struct rwbuff_t *rwbuff, int n);
inline int rwbuff_get_sendn(struct rwbuff_t *rwbuff);
inline char* rwbuff_get_sendp(struct rwbuff_t *rwbuff);

inline int rwbuff_get_left_space(struct rwbuff_t *rwbuff);
inline int rwbuff_insert_data(struct rwbuff_t *rwbuff, char* pos,char* buf, int n);
inline int rwbuff_append_data(struct rwbuff_t *rwbuff, char* buf, int n);

inline int rwbuff_append_extra_data(struct rwbuff_t *rwbuff, char* buf , int buflen);
inline struct iovec*  rwbuff_get_next_extra_data(struct rwbuff_t *rwbuff);
inline void  rwbuff_free_extra_data(struct rwbuff_t *rwbuff);

#endif

