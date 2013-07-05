#ifndef  CK_CONF_H
#define CK_CONF_H
#define CON_VALUE_TOO_LONG  5

struct  ck_conf{
	char file_name[256];
	char *buf;
	int buflen;
};

#ifdef __cplusplus
extern "C" 
{
#endif

struct ck_conf *  conf_open( char *filename);
void   conf_close( struct ck_conf  *pconf );
int    conf_get_int(struct ck_conf  *pconf , char *name ,  int  *ivalue);
int    conf_get_string(struct ck_conf  *pconf , const char *name , char* value , int  maxvaluelen);
int    conf_set_value_str(char* file_name , char* name , char* value);
int    conf_set_value_int(char* file_name , char* name , int value);
char*  conf_get_line(struct ck_conf  *pconf , char *name);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif
