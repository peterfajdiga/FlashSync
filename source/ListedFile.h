#pragma once

#include "global.h"
#include "uifstream.h"
#include "uofstream.h"

#include <string>
#include <set>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


#define ACTION_NONE   0
#define ACTION_SYNC   1
#define ACTION_DEL    2
#define ACTION_FORGET 3


class ListedFile {
    
public:
    boost::filesystem::path filename;
    std::time_t lastWriteTime;
    int action;
    hash_t hash;
    
private:
    boost::filesystem::path flashFilename;
    std::set<hash_t> oldHashes;
    std::set<id_t> owners;
    
public:
    void clean();
    boost::filesystem::path fullFlashFilename();
    
    void syncToFlash(const hash_t newHash, const boost::filesystem::path computerFilename);
    void addToFlash(const boost::filesystem::path computerFilename);
    void delFromFlash();
    
    void syncToComputer(const boost::filesystem::path computerFilename);
    void addToComputer();
    void delFromComputer(const boost::filesystem::path computerFilename);
    
    bool cmpOldHashes(const hash_t hash) const;
    bool cmpOwners(const id_t owner) const;
    void removeOwner(const id_t owner);
    void addOldHash(const hash_t hash);
    void own();
    void cleanIfSynced();
    
private:
    void delFlashFile();
    
public:
    friend uifstream& operator>> (uifstream& is, ListedFile& a);
    friend uofstream& operator<< (uofstream& os, const ListedFile& a);
    friend bool operator< (const ListedFile& a, const ListedFile& b);
};

template<typename T>
uifstream& operator>> (uifstream& is, std::set<T>& set);

template<typename T>
uofstream& operator<< (uofstream& os, const std::set<T>& set);
