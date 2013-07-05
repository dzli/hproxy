#ifndef __SCANNER_H_
#define __SCANNER_H_

typedef enum {
	SCAN_UNKNOWN = 0,
	SCAN_OK, SCAN_VIRUS,
	SCAN_FAILED, SCAN_TIMEOUT,
	SCAN_SKIPPED
} av_result;

av_result av_scan(char *filename, char* result, int len);

#endif
