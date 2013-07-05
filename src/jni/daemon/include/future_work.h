#ifndef __FUTURE_WORK_H
#define __FUTURE_WORK_H

#include "control.h"
#include "queue.h"

/* the cleanup queue holds stopped threads.  Before a thread
   terminates, it adds itself to this list.  Since the main thread is
   waiting for changes in this list, it will then wake up and clean up
   the newly terminated thread. */

typedef struct cleanup_queue {
    data_control control;
    queue cleanup;
} CleanupQueue;


/* the work_queue holds tasks for the various threads to complete. */
typedef struct work_queue {
    data_control control;
    queue work;
    int thread_count;
    int real_thread_count;
    int queue_len;
    int cur_job_count;
    int max_job_count;
    int free_job_count;
    queue free_work;
    CleanupQueue cq;
} WorkQueue;

/* I added a thread number (for debugging/instructional purposes) and
   a thread id to the cleanup node.  The cleanup node gets passed to
   the new thread on startup, and just before the thread stops, it
   attaches the cleanup node to the cleanup queue.  The main thread
   monitors the cleanup queue and is the one that performs the
   necessary cleanup. */

typedef struct cleanup_node {
    struct node *next;
    int threadnum;
    pthread_t tid;
    WorkQueue *wq;
} CleanupNode;


typedef void (*WorkFunc)(void *arg);


/* I added a job number to the work node.  Normally, the work node
   would contain additional data that needed to be processed. */
typedef struct work_node {
    struct node *next;
    int jobnum;
    WorkFunc do_work;
    void *work_arg;
} WorkNode;

void wq_future_do(WorkQueue *wq, WorkFunc do_work, void *args);
WorkQueue *wq_create(int thread_count , int queue_len);

#endif

