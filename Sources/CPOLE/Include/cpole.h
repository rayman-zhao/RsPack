#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void * CStorage_create(const char *fileName);
void CStorage_destroy(void *cstorage);
bool CStorage_open(void *cstorage);
void CStorage_close(void *cstorage);
int CStorage_result(void *cstorage);
char ** CStorage_entries(const void *cstorage, const char *path);
bool CStorage_isDirectory(const void *cstorage, const char *name);
bool CStorage_exists(const void *cstorage, const char *name);
char ** CStorage_getAllStreams(const void *cstorage, const char *storageName);

void * CStream_create(void *storage, const char *name);
void CStream_destroy(void *cstream);
char * CStream_fullName(void *cstream);
size_t CStream_size(void *cstream);
size_t CStream_read(void *cstream, unsigned char *data, size_t maxlen);
bool CStream_fail(void *cstream);

#ifdef __cplusplus
}
#endif