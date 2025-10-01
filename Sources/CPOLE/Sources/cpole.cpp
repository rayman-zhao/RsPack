#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include "../Include/cpole.h"
#include "pole.h"

using namespace POLE;

static char ** list2array(const std::list<std::string> &list)
{
    // 1. 为字符串指针数组分配内存，+1 用于存储 NULL 终止符
    char **array = (char **)malloc((list.size () + 1) * sizeof(char *));
    if (!array) {
        return NULL; // 内存分配失败
    }

    // 2. 为每个字符串分配内存并复制内容，+1 用于存储 '\0'
    int index = 0;
    for (const auto &str : list) {
        array[index] = (char *)malloc(str.size () + 1);
        
        // 分配失败时释放已分配的内存
        if (!array[index]) {
            for (int i = 0; i < index; ++i) {
                free(array[i]);
            }
            free(array);
            return NULL;
        }

        strcpy(array[index], str.c_str());
        index++;
    }

    // 3. 最后一个元素设为 NULL，作为结束标记
    array[index] = NULL;
    return array;
}

static char * str2cstr(const std::string &str)
{
    char *cstr = (char *)malloc(str.size () + 1);
    if (!cstr) {
        return NULL;
    }

    strcpy(cstr, str.c_str());
    return cstr;
}

void * CStorage_create(const char *fileName)
{
    return new Storage(fileName);
}

void CStorage_destroy(void *cstorage)
{
    delete (Storage *)cstorage;
}

bool CStorage_open(void *cstorage)
{
    return ((Storage *)cstorage)->open();
}

void CStorage_close(void *cstorage)
{
    ((Storage *)cstorage)->close();
}

int CStorage_result(void *cstorage)
{
    return ((Storage *)cstorage)->result();
}

char ** CStorage_entries(const void *cstorage, const char *path)
{
    return list2array(((Storage *)cstorage)->entries(path));
}

bool CStorage_isDirectory(const void *cstorage, const char *name)
{
    return ((Storage *)cstorage)->isDirectory(name);
}

bool CStorage_exists(const void *cstorage, const char *name)
{
    return ((Storage *)cstorage)->exists(name);
}

char ** CStorage_getAllStreams(const void *cstorage, const char *storageName)
{
    return list2array(((Storage *)cstorage)->GetAllStreams(storageName));
}

void * CStream_create(void *cstorage, const char *name)
{
    return new Stream((Storage *)cstorage, name);
}

void CStream_destroy(void *cstream)
{
    delete (Stream *)cstream;
}

char * CStream_fullName(void *cstream)
{
    return str2cstr(((Stream *)cstream)->fullName());
}

size_t CStream_size(void *cstream)
{
    return ((Stream *)cstream)->size();
}

size_t CStream_read(void *cstream, unsigned char *data, size_t maxlen)
{
    return ((Stream *)cstream)->read(data, maxlen);
}

bool CStream_fail(void *cstream)
{
    return ((Stream *)cstream)->fail();
}