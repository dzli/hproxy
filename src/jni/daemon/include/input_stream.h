#ifndef __INPUT_STREAM_H__
#define __INPUT_STREAM_H__

typedef struct input_stream_t
{
    unsigned char* inbuffer;
    unsigned int total_len;
    unsigned int windex;
    unsigned int rindex;
    unsigned int avail_len;

} input_stream;

enum { ISTREAM_OK = 0, ISTREAM_ERROR_MEM = 1, ISTREAM_ERROR_PARAM };

#ifdef __cplusplus
extern "C" 
{
#endif

unsigned int istream_datalen(input_stream* pstream);
unsigned char*  istream_buffer(input_stream* pstream);
int istream_init(input_stream* pstream);
int istream_initx(input_stream* pstream, int size);
int istream_reset(input_stream* pstream);
int istream_destroy(input_stream* pstream);
int istream_write(input_stream* pstream, unsigned char* buff, unsigned int len);
int istream_read(input_stream* pstream, unsigned char* buff, unsigned int maxlen);
char* istream_strerror(input_stream* pstream, int code);


#ifdef __cplusplus
}  /* end extern "C" */
#endif



#endif

