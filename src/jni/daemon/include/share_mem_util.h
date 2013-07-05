#ifndef __FCM_SHARE_MEM_UTIL_H__
#define __FCM_SHARE_MEM_UTIL_H__

#ifdef __cplusplus
extern "C" 
{
#endif

/*
    functions for FCM share memory
*/
int FSM_init(unsigned int key, unsigned int size);
int FSM_attach(unsigned int key, unsigned int size, void** outmem);
int FSM_detach(void* share_mem);
int FSM_destroy(unsigned int key, unsigned int size);

/*
    functions for FCM lock
*/
int FSM_init_lock(unsigned int key);
int FSM_destroy_lock(unsigned int key);
int FSM_lock(unsigned int key);
int FSM_unlock(unsigned int key);


#ifdef __cplusplus
}  /* end extern "C" */
#endif


/*
    atomic add.
*/
static	inline void atomic_addl(volatile int *addr, int val)
{
	__asm__ volatile("lock;"
		"addl %1, %0;"
		:"=m"(*(int *)addr)
		:"ir"(val), "m"(*(int *)addr)
		:"memory");
}


#endif

