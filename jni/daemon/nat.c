#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "fctl.h"
//#include <sys/sysctl.h>

extern struct fctl_conf_t g_conf;

static int ipfw_rule_number = 10;

#ifdef  __APPLE__
static int net_inet_ip_scopedroute;

static  int set_ip_scopedroute(int enable)
{
    size_t  olen, nlen;
    int     oldval;

    olen = sizeof(oldval);
    nlen = sizeof(enable);
    if (sysctlbyname("net.inet.ip.scopedroute",
                &oldval, &olen, &enable, nlen) < 0) {
        return  (-1);
    }

    return  (oldval);
}

#endif  /* __APPLE__ */

#ifdef	__linux__

static int do_command(char* filename , char* argv[]){
    pid_t   pid;

    if ((pid = fork()) < 0) {
        return -1;
    } else if (pid == 0) { 
        execv(filename , argv);
	 exit(0);	
    }
    if (waitpid(pid, NULL, 0) < 0){
        return -2;
    }
    return 0;
}

#endif	/* __linux__ */

int nat_disable_port(int src_port , int dst_port ){
#ifdef __linux__
#if 0
char* argv[]={"iptables", "-t" , "nat" , "-I", "OUTPUT", "1", "-p", "tcp", "--dport", NULL,
	 	              "--sport", NULL , "-j", "ACCEPT", (char*)0};
    
    char str_dst_port[10];
    char str_src_port[10];  
    snprintf(str_src_port, sizeof(str_src_port) , "%d", src_port);
    snprintf(str_dst_port, sizeof(str_dst_port) , "%d", dst_port);
    argv[9] = str_dst_port;
    argv[11] = str_src_port;	

    do_command(g_conf.iptables, argv);
#endif
#endif
    return (0); /* clear warning */
}

int nat_enable_port(int src_port , int dst_port ){
#ifdef __linux__
#if 0
char* argv[]={"iptables", "-t" , "nat" , "-D", "OUTPUT",  "-p", "tcp", "--dport", NULL,
	 	              "--sport", NULL , "-j", "ACCEPT", (char*)0};
    char str_dst_port[10];
    char str_src_port[10];  
    snprintf(str_src_port, sizeof(str_src_port) , "%d", src_port);
    snprintf(str_dst_port, sizeof(str_dst_port) , "%d", dst_port);
    argv[8] = str_dst_port;
    argv[10] = str_src_port;	

    do_command(g_conf.iptables, argv);
#endif
#endif
    return (0); /* clear warning */
}

void init_iptables_nat_rules(){
    char cmd[128];
#ifdef __linux__
    snprintf(cmd , sizeof(cmd), "%s -t nat -F", g_conf.iptables);
	system(cmd);
    //snprintf(cmd , sizeof(cmd), "%s -t nat -A OUTPUT -p tcp --sport 61000:65535 -j ACCEPT", g_conf.iptables);
	snprintf(cmd , sizeof(cmd), "%s -t nat -A OUTPUT -m owner --uid-owner 0 -j ACCEPT", g_conf.iptables);
	system(cmd);
#endif

#ifdef __APPLE__
    /*
     * set the sysctl param "net.inet.ip.scopedroute" to 0,
     * or the ipfw forwarding does not work.
     */
    net_inet_ip_scopedroute = set_ip_scopedroute(0);
    if (net_inet_ip_scopedroute < 0) {
        net_inet_ip_scopedroute = 0;
    }

    snprintf(cmd, sizeof(cmd), 
        "%s add %d set 10 allow ip from any to any via lo0 ",
        g_conf.iptables,
        ipfw_rule_number++);
	system(cmd);
    
    snprintf(cmd, sizeof(cmd), 
        "%s add %d set 10 allow ip from me 61000-65535 to not me  ",
        g_conf.iptables,
        ipfw_rule_number++);
	system(cmd);

#endif
}

void create_iptables_nat_rule(int dest_port , int redirect_port){
    char cmd[128];
#ifdef __linux__    
    //#define RULE_TEMPLATE "%s -t nat -A OUTPUT -p tcp -d ! 127.0.0.1 --dport %d -j REDIRECT --to-ports %d"
    #define RULE_TEMPLATE "%s -t nat -A OUTPUT -p tcp --dport %d -j REDIRECT --to-ports %d"
    snprintf(cmd , sizeof(cmd), RULE_TEMPLATE ,
         g_conf.iptables, dest_port, redirect_port);
    printf("%s\n", cmd);

#endif

#ifdef __APPLE__

#ifdef HTTP_RECV_ALL_TRAFFIC
    #define RULE_TEMPLATE "%s add %d set 10 fwd  127.0.0.1,%d tcp from any to any out "
    snprintf(cmd , sizeof(cmd), RULE_TEMPLATE ,
         g_conf.iptables, ipfw_rule_number++, redirect_port);
#else
    #define RULE_TEMPLATE "%s add %d set 10 fwd 127.0.0.1,%d tcp from me to not me dst-port %d"
    snprintf(cmd , sizeof(cmd), RULE_TEMPLATE ,
         g_conf.iptables, ipfw_rule_number++, redirect_port, dest_port);
#endif

#endif    
    
    system(cmd);
}


void destroy_iptables_nat_rules(){

#ifdef __linux__
    char cmd[128];
    snprintf(cmd , sizeof(cmd), "%s -t nat -F", g_conf.iptables);
	system(cmd);
#endif   

#ifdef __APPLE__
    char cmd[128];
    set_ip_scopedroute(net_inet_ip_scopedroute);
    snprintf(cmd, sizeof(cmd), 
        "%s delete  set 10  ",
        g_conf.iptables);
    system(cmd);
    
#endif

}

#if 0
int main(){
    nat_disable_port( 1111,110);	
    return 0;
}
#endif

