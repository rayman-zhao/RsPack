#pragma once

#include <memory>
#include <vector>
#include "pole.h"

class CStorage {
    friend class CStream;
    
public:
    enum class Result { Ok, OpenFailed, NotOLE, BadOLE, UnknownError };
    
    CStorage(const char *filename)
        : storage(std::make_shared<POLE::Storage>(filename)) {
    }
    ~CStorage() {
    }
    
    bool open() const {
        return storage->open();
    }
    void close() const {
        storage->close();
    }
    int result() const {
        return storage->result();
    }
    std::vector<std::string> entries(const std::string &path = "/") const {
        auto list = storage->entries(path);
        return std::vector(list.begin(), list.end());
    }
    bool isDirectory(const std::string &name) const {
        return storage->isDirectory(name);
    }
    bool exists(const char *name) const {
        return storage->exists(name);
    }
    std::list<std::string> GetAllStreams(const std::string &storageName) const {
        return storage->GetAllStreams(storageName);
    }
    
private:
    std::shared_ptr<POLE::Storage> storage;
};

class CStream {
public:
    CStream(const CStorage &storage, const char *name, bool bCreate = false, POLE::int64 streamSize = 0)
    : stream(std::make_shared<POLE::Stream>(storage.storage.get(), name, bCreate, streamSize)) {
    }
    ~CStream() {
    }
    
    std::string fullName() const {
        return stream->fullName();
    }
    POLE::uint64 size() const {
        return stream->size();
    }
    POLE::uint64 read(unsigned char *data, POLE::uint64 maxlen) const {
        return stream->read(data, maxlen);
    }
    bool fail() const {
        return stream->fail();
    }

private:
    std::shared_ptr<POLE::Stream> stream;
};
