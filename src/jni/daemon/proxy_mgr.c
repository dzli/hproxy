
#include "fctl.h"
#include <signal.h>
#include <unistd.h>

int pc_add_proxy(struct proxy_container_t* pc, struct fctl_proxy_t* proxy){
    
    if (pc->index >= (MAX_PROXY_SIZE-1))
        return -1;
    pc->proxies[pc->index++] = proxy;
    
    return 0;        
}


int pc_run_all(struct proxy_container_t* pc){
    int i =0;
    
    for(i = 0 ; i < pc->others_count; i++ ){
       pid_t   pid;

       if ((pid = fork()) < 0) {
           pid = 0;
       } else if (pid == 0) { 
           int ret = 0;
           if (pc->others[i].arg1[0] == 0)
               ret = execl(pc->others[i].filename ,pc->others[i].filename, (char*)0);
           else
               ret = execl(pc->others[i].filename ,pc->others[i].filename, pc->others[i].arg1, (char*)0);
           ERRLOG("execute %s error: %s\n", pc->others[i].filename, strerror(errno) );
           exit(0);  	
       }
       printf("%s started\n",pc->others[i].filename);
#ifdef __linux__
       sleep(1);
#endif
       pc->others[i].pid = pid;
    }
    
    for(i = 0 ; i < pc->index; i++ ){
        pc->proxies[i]->run( pc->proxies[i]);
    }
    
    return 0 ;
}

int pc_kill_all(struct proxy_container_t* pc, int signo){
    int i =0;
    
    for(i = 0 ; i < pc->index; i++ ){
        if (pc->proxies[i]->pid >0)
        kill(pc->proxies[i]->pid , signo);
    }
    
    for(i = 0 ; i < pc->others_count; i++ ){
        if (pc->others[i].pid >0)
            kill(pc->others[i].pid , signo);
    }
    return 0 ;
}

int pc_restart_proxy(struct proxy_container_t* pc, pid_t proxy_pid){
   int i =0;
    
    for(i = 0 ; i < pc->index; i++ ){
        if (pc->proxies[i]->pid == proxy_pid){
            pc->proxies[i]->run( pc->proxies[i]);
            break;
        }
    }
    
    return 0 ;
}

int pc_remove_pid(struct proxy_container_t* pc, pid_t proxy_pid){
   int i =0;
    
    for(i = 0 ; i < pc->index; i++ ){
        if (pc->proxies[i]->pid == proxy_pid){
            pc->proxies[i]->pid = 0;
            break;
        }
    }
    
    for(i = 0 ; i < pc->others_count; i++ ){
        if (pc->others[i].pid == proxy_pid ){
                pc->others[i].pid = 0;
            break;
        }
    }

    return 0 ;
}

int pc_add_others(struct proxy_container_t* pc, int daemon,  char* filename, char* arg1){
        
    strncpy(pc->others[pc->others_count].filename, filename , MAX_FILE_NAME-1);
    pc->others[pc->others_count].daemon = daemon;
    if (arg1 != NULL)
        strncpy(pc->others[pc->others_count].arg1, arg1 , sizeof(pc->others[pc->others_count].arg1)-1);
    
    pc->others_count++;
    return 0;        
}

int pc_stop_others(struct proxy_container_t* pc, char* filename ){
    int i = 0;
    
     for(i = 0 ; i < pc->others_count; i++ ){
        if ( strcmp(pc->others[i].filename , filename) == 0 ){
            if ( pc->others[i].pid > 0){
                kill(pc->others[i].pid , SIGINT);
                pc->others[i].pid = 0;
            }
            break;
            
        }
    }
    return 0;
}


int pc_restart_others(struct proxy_container_t* pc, pid_t pid ){
    int i = 0;
    for(i = 0 ; i < pc->others_count; i++ ){
        if ( pc->others[i].pid  == pid ){
            pid_t   pid;
            if ((pid = fork()) < 0) {
                pid = 0;
            } else if (pid == 0) { 
                int ret = 0;
                if (pc->others[i].arg1[0] == 0)
                    ret = execl(pc->others[i].filename ,pc->others[i].filename, (char*)0);
                else
                    ret = execl(pc->others[i].filename ,pc->others[i].filename, pc->others[i].arg1, (char*)0);
                ERRLOG("execute %s error: %s\n", pc->others[i].filename, strerror(errno) );
                exit(0);  	
            }
            pc->others[i].pid = pid;
            break;
        }
    }
  
    return 0;
}

/*
int pc_start_others(struct proxy_container_t* pc, char* filename ){
    int i = 0;
    int found = 0;

AGAIN:    
    for(i = 0 ; i < pc->others_count; i++ ){
        if ( strcmp(pc->others[i].filename , filename) == 0 ){
            found = 1;
            if ( pc->others[i].pid <= 0){
                pid_t   pid;
                if ((pid = fork()) < 0) {
                    pid = 0;
                } else if (pid == 0) { 
                    int ret = execl(pc->others[i].filename ,pc->others[i].filename, pc->others[i].arg1, (char*)0);
                    if (ret < 0)
                        ERRLOG("execute %s error: %s\n", pc->others[i].filename, strerror(errno) );
                    exit(0);  	
                }
                pc->others[i].pid = pid;
       
            } 
             
            break;
        }
    }
    if (!found){
        pc_add_others(pc, filename, NULL);
        goto AGAIN;
     }
    
}
*/

