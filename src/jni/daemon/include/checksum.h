#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

//In release build, we use this C version which is faster than
//the assemblly version
//len is the IP header size in 4-byte words, not in bytes
static __inline unsigned short fast_csum(char *hdr, unsigned int len)
{
	unsigned short *buffer = (unsigned short*)hdr;
    unsigned long cksum = 0;

	/* If change the size type from "int" to "unsigned int", it will become
	   much slower. VC optimization is a bit tricky */
	int size = len;

    while (size > 1) {
        cksum += *buffer++;
        size -= sizeof(unsigned short);
    }
    if (size != 0)
		cksum += *(unsigned char *)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    return (unsigned short)(~cksum);
}


#endif
