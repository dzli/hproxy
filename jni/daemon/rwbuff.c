#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rwbuff.h"

inline char* rwbuff_get_readp(struct rwbuff_t *rwbuff){
    return rwbuff->readp;
};

inline int rwbuff_get_readn(struct rwbuff_t *rwbuff){
    return (rwbuff->writep - rwbuff->readp);
}

inline int rwbuff_read(struct rwbuff_t *rwbuff, int n){
    rwbuff->readp += n;
 /*   if (rwbuff->readp == rwbuff->writep){
        rwbuff->readp = rwbuff->buf;
        rwbuff->writep = rwbuff->buf;
    }*/
    return 0;
}


inline char* rwbuff_get_sendp(struct rwbuff_t *rwbuff){
    return rwbuff->sendp;
};

inline int rwbuff_get_sendn(struct rwbuff_t *rwbuff){
    return (rwbuff->readp - rwbuff->sendp);
}

inline int rwbuff_send(struct rwbuff_t *rwbuff, int n){
    rwbuff->sendp += n;
    if ( rwbuff->sendp == rwbuff->readp && rwbuff->readp == rwbuff->writep){
        rwbuff->readp = rwbuff->buf;
        rwbuff->writep = rwbuff->buf;
        rwbuff->sendp = rwbuff->buf;
    }
    return 0;
}

inline char* rwbuff_get_writep(struct rwbuff_t *rwbuff){
    return rwbuff->writep;
}

inline int rwbuff_get_writen(struct rwbuff_t *rwbuff){
    return (rwbuff->buf_len - (rwbuff->writep - rwbuff->buf) );
}

inline int rwbuff_write(struct rwbuff_t *rwbuff, int n){
   if (n > rwbuff_get_writen(rwbuff)) {
       n =  rwbuff_get_writen(rwbuff);       
   }
    rwbuff->writep += n;
}

inline int rwbuff_get_left_space(struct rwbuff_t *rwbuff){
   
   return (RWBUFF_PRESERVE_LEN +rwbuff->buf_len + RWBUFF_LEN - 
            (rwbuff->writep - rwbuff->buf) );

}

inline int rwbuff_insert_data(struct rwbuff_t *rwbuff, char* pos,char* buf, int n){
    if (rwbuff_get_left_space(rwbuff) < n )
        n = rwbuff_get_left_space(rwbuff);
            
    if ( rwbuff->writep - pos > 0)
        memmove(pos + n , pos , rwbuff->writep - pos);
    
    memcpy(pos , buf , n);
   
    rwbuff->writep += n;
    
    return 0;
}

inline int rwbuff_append_data(struct rwbuff_t *rwbuff, char* buf, int n){
    char * pos = rwbuff->writep;
    
    return rwbuff_insert_data(rwbuff , pos , buf , n);
}

/*
inline int rwbuff_append_extra_data(struct rwbuff_t *rwbuff, char* buf , int buflen){
    if (rwbuff->extra_data_count >= sizeof(rwbuff->extra_data)/sizeof(rwbuff->extra_data[0])){
        return -1;
    }

    rwbuff->extra_data[rwbuff->extra_data_count].iov_base = buf;
    rwbuff->extra_data[rwbuff->extra_data_count].iov_len = buflen;

    rwbuff->extra_data_count++;

    return 0;
}
*
inline struct iovec*  rwbuff_get_next_extra_data(struct rwbuff_t *rwbuff){
    if (rwbuff->extra_data_usage == 0 || rwbuff->extra_data_count == 0){    
        return NULL;
    }
    struct iovec* ret = &rwbuff->extra_data[rwbuff->extra_data_read_count++];

    if (rwbuff->extra_data_read_count  == rwbuff->extra_data_count){
        rwbuff->extra_data_usage = 0;
        rwbuff->extra_data_read_count = 0;
        rwbuff->extra_data_count = 0;
    }

    return ret;
}

inline void  rwbuff_free_extra_data(struct rwbuff_t *rwbuff){
    if ( rwbuff->extra_data_count == 0){    
        return ;
    }
    int i = 0;
    for(i = 0 ; i < rwbuff->extra_data_read_count; i++){
        free(rwbuff->extra_data[i].iov_base);
    }
    rwbuff->extra_data_usage = 0;
    rwbuff->extra_data_read_count = 0;
    rwbuff->extra_data_count = 0;
  
}
*/
inline struct rwbuff_t * rwbuff_new(){
    struct rwbuff_t * rwbuff = NULL;
    rwbuff = (struct rwbuff_t*)malloc( sizeof(struct rwbuff_t));
    if (rwbuff == NULL){
        return NULL;
    }
    memset(rwbuff , 0 , sizeof(struct rwbuff_t));
    rwbuff->buf_len = RWBUFF_LEN;
    rwbuff->buf_default_len = RWBUFF_LEN;
    rwbuff->readp = rwbuff->buf;
    rwbuff->writep = rwbuff->buf;
    rwbuff->sendp = rwbuff->buf;
    //rwbuff->extra_file = -1;
    cp_init(&rwbuff->cache_pool);
    
    return rwbuff;
};

inline int rwbuff_init(struct rwbuff_t * rwbuff){
    if (rwbuff == NULL){
        return -1;
    }
    memset(rwbuff , 0 , sizeof(struct rwbuff_t));
    rwbuff->buf_len = RWBUFF_LEN;
    rwbuff->buf_default_len = RWBUFF_LEN;
    rwbuff->readp = rwbuff->buf;
    rwbuff->writep = rwbuff->buf;
    rwbuff->sendp = rwbuff->buf;
    //rwbuff->extra_file = -1;
    cp_init(&rwbuff->cache_pool);

    return 0;
};

inline int rwbuff_init_ptr(struct rwbuff_t * rwbuff){
    if (rwbuff == NULL){
        return -1;
    }
    rwbuff->readp = rwbuff->buf;
    rwbuff->writep = rwbuff->buf;
    rwbuff->sendp = rwbuff->buf;

    return 0;
};

inline int rwbuff_rewind_writep(struct rwbuff_t * rwbuff){
    if (rwbuff == NULL){
        return -1;
    }
    rwbuff->writep = rwbuff->readp;
   
    return 0;
};

