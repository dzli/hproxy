#ifndef __SHARE_DATA_H__
#define __SHARE_DATA_H__

#include <time.h>

struct fct_global_data{
    time_t en_sig_update_time;
};

#define FCT_GLOBAL_DATA_SHARE_KEY     0x1111 

#ifdef __cplusplus
extern "C" 
{
#endif

struct fct_global_data* sd_get_global_data();
void sd_free_global_data();

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif

