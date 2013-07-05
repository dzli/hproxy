#ifndef MMAP_H
#define MMAP_H

void * start_mmap(char* filename, int* pfilelen);
void end_mmap(void *start, int filelen);

#endif

