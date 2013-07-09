#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __linux__
//#include <error.h>
#endif
#include "../include/input_stream.h"

unsigned int istream_datalen(input_stream* pstream)
{
    if (pstream == NULL)
        return 0;

    return pstream->windex - pstream->rindex;
}

unsigned char* istream_buffer(input_stream* pstream)
{
    if (pstream == NULL)
        return 0;

    return pstream->inbuffer + pstream->rindex;
}

int istream_init(input_stream* pstream)
{
    if (pstream == NULL)
        return ISTREAM_ERROR_PARAM;

    memset(pstream, 0, sizeof(*pstream));

    return ISTREAM_OK;
}

int istream_initx(input_stream* pstream, int size)
{
    if (pstream == NULL)
        return ISTREAM_ERROR_PARAM;

    memset(pstream, 0, sizeof(*pstream));

    //assert(pstream->inbuffer == NULL);
    pstream->inbuffer = (unsigned char*)malloc(size);
    if (pstream->inbuffer == NULL)
        return ISTREAM_ERROR_MEM;
    pstream->total_len = size;
    pstream->windex = 0;
    pstream->rindex = 0;
    pstream->avail_len = size;


    return ISTREAM_OK;
}

int istream_reset(input_stream* pstream)
{
    if (pstream == NULL)
        return ISTREAM_ERROR_PARAM;

    pstream->rindex = 0;
    pstream->windex = 0;
    pstream->avail_len = pstream->total_len;

    return ISTREAM_OK;
}

int istream_destroy(input_stream* pstream)
{
    if (pstream != NULL && pstream->inbuffer != NULL)
        free(pstream->inbuffer);

    return ISTREAM_OK;
}

#define MIN_INC_BLOCK  256

int  istream_write(input_stream* pstream, unsigned char* buff, unsigned int len)
{
    if (pstream == NULL || buff == NULL || len <=0)
        return ISTREAM_ERROR_PARAM;

    if (pstream->inbuffer == NULL)
    {
        pstream->inbuffer = (unsigned char*)malloc(len);
        if (pstream->inbuffer == NULL)
            return ISTREAM_ERROR_MEM;
        pstream->total_len = len;
        pstream->windex = 0;
        pstream->rindex = 0;
        pstream->avail_len = len;
    }
    else
    {
        if (pstream->rindex != 0)
        {
            unsigned int datalen = (unsigned int)(pstream->windex - pstream->rindex);
            memcpy(pstream->inbuffer, pstream->inbuffer + pstream->rindex, datalen);
            pstream->rindex = 0;
            pstream->windex = pstream->rindex + datalen;
            pstream->avail_len = pstream->total_len - datalen;
        }

        if (pstream->avail_len < len)
        {
            unsigned int inc_size =  (len > MIN_INC_BLOCK) ? len : MIN_INC_BLOCK;
            pstream->inbuffer = (unsigned char*)realloc(pstream->inbuffer, pstream->total_len + inc_size);
            if (pstream->inbuffer == NULL)
                return ISTREAM_ERROR_MEM;

            pstream->total_len = pstream->total_len + inc_size;
        }
    }


    memcpy(pstream->inbuffer + pstream->windex , buff, len);
    pstream->windex += len;
    pstream->avail_len = pstream->total_len - pstream->windex ;

    return ISTREAM_OK;
}

int  istream_read(input_stream* pstream, unsigned char* buff, unsigned int maxlen)
{
    int readlen = (maxlen <= (pstream->windex - pstream->rindex))?maxlen : (pstream->windex - pstream->rindex); 
    if (buff != NULL && readlen != 0)
    {
        memcpy(buff, pstream->inbuffer + pstream->rindex, readlen);
    }
    pstream->rindex += readlen;

    return readlen;
}

char* istream_strerror(input_stream* pstream, int code)
{
    switch (code) 
    {
        case ISTREAM_ERROR_MEM:
            return "Memory error";
        case ISTREAM_ERROR_PARAM:
            return "Param error";
        default:
            return "Unknown error";
    }
    return "";
}

#if 0
int main()
{
    input_stream istream;
    istream_reset(&istream);

    istream_write(&istream, "1234567890", 10);
    char buff[128];
    
    int n = istream_read(&istream, buff, 128);
    printf("n = %d\n", n);
    buff[n] = 0;
    printf("buff:%s\n", buff);

    istream_write(&istream, "1234567890", 10);
    n = istream_read(&istream, buff, 3);
    printf("n = %d\n", n);
    buff[n] = 0;
    printf("buff:%s\n", buff);
    
    n = istream_read(&istream, buff, 4);
    printf("n = %d\n", n);
    buff[n] = 0;
    printf("buff:%s\n", buff);


    return 0;
}

#endif

