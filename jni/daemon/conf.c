#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

#include "../include/conf.h"

#define FatalPrintError printf

#if 1
#define DebugMessage printf
#else
#define DebugMessage(format, args...)
#endif

#ifdef USE_MMAP
void *  __start_mmap(char* filename, int* pfilelen){
	int fd;
	void *start;
	struct stat sb;
	fd=open(filename,  O_RDONLY);
	if (!fd){
		//FatalPrintError("can not open file:%s",filename);
		return NULL;
	}
	fstat(fd,&sb);
	
	*pfilelen = sb.st_size;

	if (sb.st_size == 0){
			return NULL;
	}
	
	start=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	if( start== MAP_FAILED ){ 
		//FatalPrintError("size = %d", sb.st_size);
		//perror("");
		close(fd);
		//FatalPrintError("mmap error!\n");
		return NULL;
	}
	close(fd);
	return start;
};

void  __end_mmap(void *start, int filelen){
	munmap(start,filelen);
}

#else

//because mmap can not work under wmware fusion in snow leopard, 
//use the following workawound to replace mmap

void *  __start_mmap(char* filename, int* pfilelen)
{
    int fd = -1;
    void *start = NULL, *filebuf = NULL;
    struct stat sb;
    fd=open(filename,  O_RDONLY);
    if (fd < 0){
        return NULL;
    }
    fstat(fd,&sb);

    *pfilelen = sb.st_size;

    if (sb.st_size == 0){
        goto out;
    }

    filebuf = malloc(sb.st_size);
    if (filebuf == NULL){
        goto out;
    }

    int n = read(fd, filebuf, sb.st_size);
    if (n != sb.st_size){
        free(filebuf);
        goto out;
    }

    start = filebuf;
out:
    if (fd >= 0)
        close(fd);
    return start;
};

void  __end_mmap(void *start, int filelen)
{
    if (start)
        free(start);
}

#endif


char* __strstr(char* buf , int buflen , const char*str){
		int i = 0;
		int str_len = strlen(str);
		int maxi = buflen - str_len ;
		while (i <= maxi && strncasecmp(buf+i, str, str_len) ) i++;
		if (i > maxi) return NULL;
		return buf+i;
}

int static __line_char_count(char* buf){
		int i =0;
		while(buf[i] != '\r' && buf[i] != '\n' && buf[i]!= '\0') i++;
		return i;
}

static   int __go_to_next_line(char** pbuf , int  buflen){
		int i = 0;
		
		while( i <= buflen &&  **pbuf!= '\r'  &&  **pbuf!= '\n'  &&  **pbuf!= '\0')  {
				(*pbuf)++;
				i++;
		}
		if  (i > buflen) return 1;
	//	printf(" buflen = %d , i =%d\n", buflen , i);
		while  ( i <= buflen  && (**pbuf ==  '\n'  ||  **pbuf  ==  '\r') )   {
				(*pbuf)++;
				i++;
		}
		if  (i > buflen) return 1;

		return 0;
}

int   conf_get_string(struct ck_conf  *pconf , const char *name , char* value , int  maxvaluelen){
		int err = 0;
		char *start_pos  ;
		char * tmp_ptr ;
		int  real_value_len = 0;
		int seen_quote = 0;
	
		#define LEFT_LEN    pconf->buflen - (long) start_pos + (long) pconf->buf 

		
		start_pos = pconf->buf;
		while( LEFT_LEN >0 ){
				
				start_pos = __strstr(start_pos  , pconf->buflen , name);

                if (start_pos != pconf->buf){
                    char prec = *(start_pos - 1);
                    if (prec != ' ' && prec != '\r' && prec != '\n' && prec != '\t'){
						err = __go_to_next_line(&start_pos,  LEFT_LEN);
						if (err)  return 1;
                    }
                }

				if (!start_pos) return 1;

				//  # is comment  , check if  # in this line  
				tmp_ptr = start_pos;
				while( tmp_ptr != pconf->buf &&  *tmp_ptr != '\r'  && *tmp_ptr != '\n' && *tmp_ptr != '#')  tmp_ptr--;
				
				if (*tmp_ptr == '#'){
						err = __go_to_next_line(&start_pos,  LEFT_LEN);
						if (err)  return 1;
				}
				else {
						break;
				}
		}
		
		//got here , I kown the keyword found.
		start_pos +=  strlen(name);

		int line_len = __line_char_count(start_pos);
		tmp_ptr =  __strstr(start_pos ,line_len , "=");

		if (tmp_ptr){
			  start_pos += 1;
		}

		
		while ( *start_pos == ' ' || *start_pos == '\t')  start_pos++;

		if ( *start_pos == '"')  {
			seen_quote = 1;
			start_pos++;
		};
		

		while( LEFT_LEN > 0 &&   *start_pos !=  '\r'  &&   *start_pos != '\n' &&   *start_pos != '#' ){
				if ( real_value_len  >  maxvaluelen - 1 ) break;
				if ( seen_quote ){
						if ( *start_pos =='"') break;
				}else{
						if  ( *start_pos ==  ' ' ) break;
				}

				value[real_value_len] = *start_pos;
				start_pos++;
				real_value_len++;
		}

		value[real_value_len] = 0;		
		if ( real_value_len  >  maxvaluelen - 1 ) 
			return CON_VALUE_TOO_LONG;

		return 0 ;

}



int conf_get_int(struct ck_conf  *pconf , char *name ,  int  *ivalue){
		char value[50];
		int ret = 0;
		ret = conf_get_string(pconf , name , value , 50);

		if (ret) return ret;

		int i = atoi(value);
		*ivalue = i;

		return 0;

}

char*   conf_get_line(struct ck_conf  *pconf , char *name){
		int err = 0;
		char *start_pos  ;
		char * tmp_ptr ;
		
		#define LEFT_LEN    pconf->buflen - (long) start_pos + (long) pconf->buf 

		
		start_pos = pconf->buf;
		while( LEFT_LEN >0 ){
				
				start_pos = __strstr(start_pos  , pconf->buflen , name);
				
				if (!start_pos) return NULL;

				//  # is comment  , check if  # in this line  
				tmp_ptr = start_pos;
				while( tmp_ptr != pconf->buf &&  *tmp_ptr != '\r'  && *tmp_ptr != '\n' && *tmp_ptr != '#')  tmp_ptr--;
				
				if ( *tmp_ptr == '#'){
						err = __go_to_next_line(&start_pos,  LEFT_LEN);
						if (err)  return NULL;
				}
				else {
						break;
				}
		}
		
		//got here , I kown the keyword found.
		int line_len = __line_char_count(start_pos);
        char* str = malloc(line_len + 1);
        memcpy(str, start_pos, line_len);
        str[line_len] = 0;

        return str ;

}

struct ck_conf *  conf_open( char *filename){
		struct ck_conf  *pconf = (struct ck_conf *)malloc(sizeof(struct ck_conf));
		if (!pconf) return NULL;

        strncpy(pconf->file_name, filename , sizeof(pconf->file_name - 1));
                
		pconf->buf = (char*)__start_mmap(filename ,  &pconf->buflen);

		if  (!pconf->buf) return NULL;
		
		return pconf;
}

void   conf_close( struct ck_conf  *pconf ){
		if (!pconf) return ;
		__end_mmap(pconf->buf , pconf->buflen);
		free(pconf);
}

static int conf_set_value(char* file_name , char* name , char* value, int addquote){
    FILE  *r_stream = NULL, *w_stream = NULL ;
    char tmp_name[256];
    int found = 0;
    
    snprintf(tmp_name ,  sizeof(tmp_name) - 1, "%s.tmp", file_name);
    w_stream = fopen(tmp_name, "w");
    if (w_stream == 0){
        return -1;
    }
    r_stream = fopen(file_name, "r" );
    if (r_stream == NULL)
        return -2;

    char line[1024];
    char new_line[1024];
    
    if (addquote)
        snprintf(new_line , sizeof(new_line), "%s \"%s\"\n", name , value); 
    else
        snprintf(new_line , sizeof(new_line), "%s %s\n", name , value); 
    
    while ( fgets(line , sizeof(line), r_stream) ) {
        char* start = line;
        while ( start < (line + sizeof(line)) && (*start == ' ' || *start == '\t')) start++;
        int namelen = strlen(name);
        if (strncmp(start , name, namelen) == 0 && (start[namelen] == ' ' || start[namelen] == '\t')){
		    fputs(new_line , w_stream);
		    found = 1;
        }else{
            fputs(line , w_stream);
        }
    }
    
    fclose(w_stream);
    fclose(r_stream);
    
    int err = rename(tmp_name , file_name);        
    
    if (found && err == 0)
        return 0;
    else
        return -3;    
}

int conf_set_value_str(char* file_name , char* name , char* value){
    return conf_set_value(file_name, name, value, 1);
}

int conf_set_value_int(char* file_name , char* name , int value){

    char str_value[1024];

    snprintf(str_value , sizeof(str_value), "%d", value);

    return conf_set_value(file_name , name , str_value, 0);

}

#if __TEST__
int main(int argc , char* argv[]){
	char filename[] = "/usr/local/forticlient/conf/forticlient.conf";
	char value[100];
    if (argc < 3){
        printf("usage: %s name value\n", argv[0]);
		exit(0);
	}
	//printf("name=%s, value=%s\n", argv[1], argv[2]);
	int ret = conf_set_value_str(filename, argv[1], argv[2]);
	
}
#endif
