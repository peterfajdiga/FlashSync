#include "ListedFile.h"

#include "ConfigComputer.h"
#include "ConfigFlash.h"
#include "FileOperations.h"

#include <assert.h>

#include <iostream>

namespace fs = boost::filesystem;


extern ConfigComputer* computer;
extern ConfigFlash* flash;
extern bool verbose;

void ListedFile::clean() {
    filename = "";
    flashFilename = "";
    action = ACTION_NONE;
    hash = 0;
    oldHashes.clear();
    owners.clear();
}

fs::path ListedFile::fullFlashFilename() {
    return flash->dir / flashFilename;
}


// computer file is newer than flash
void ListedFile::syncToFlash(const hash_t newHash, const fs::path computerFilename) {
    if (verbose) {
        std::cout << "sync to flash:     " << filename << std::endl;
    }
    if (flashFilename.empty()) {  // TODO: test condition
        assert(action != ACTION_SYNC);
        flashFilename = flash->getFreeFilename();
    }
    if (FileOperations::copy_file(computerFilename, fullFlashFilename(), fs::copy_option::overwrite_if_exists)) {
        lastWriteTime = fs::last_write_time(computerFilename);
        action = ACTION_SYNC;
        addOldHash(hash);
        hash = newHash;
        owners.clear();
        own();
    }  // else user chose to skip file
}

void ListedFile::addToFlash(const fs::path computerFilename) {
    filename = relative(computerFilename, computer->syncDir);
    lastWriteTime = fs::last_write_time(computerFilename);
    if (verbose) {
        std::cout << "add to flash:      " << filename << std::endl;
    }
    assert(action != ACTION_SYNC);
    flashFilename = flash->getFreeFilename();
    if (FileOperations::copy_file(computerFilename, fullFlashFilename())) {
        action = ACTION_SYNC;
        hash = FileOperations::hashFile(computerFilename);
        own();
    } else {
        // user chose to skip file
        action = ACTION_FORGET;
    }
}

void ListedFile::delFromFlash() {
    // mark it for deletion, if not already
    if (action != ACTION_DEL) {
        if (verbose) {
            std::cout << "del from flash:    " << filename << std::endl;
        }
        lastWriteTime = time(NULL);
        action = ACTION_DEL;
        addOldHash(hash);
        owners.clear();
    }
    own();
    
    delFlashFile();
}


// computer file is older than flash
void ListedFile::syncToComputer(const fs::path computerFilename) {
    if (verbose) {
        std::cout << "sync to computer:  " << filename << std::endl;
    }
    create_directories(computerFilename.parent_path());
    if (FileOperations::copy_file(fullFlashFilename(), computerFilename, fs::copy_option::overwrite_if_exists)) {
        own();
        fs::last_write_time(computerFilename, lastWriteTime);
    }  // else user chose to skip file
}
void ListedFile::addToComputer() {
    if (verbose) {
        std::cout << "add to computer:   " << filename << std::endl;
    }
    fs::path computerFilename = computer->syncDir / filename;
    create_directories(computerFilename.parent_path());
    if (FileOperations::copy_file(fullFlashFilename(), computerFilename)) {
        own();
        fs::last_write_time(computerFilename, lastWriteTime);
    }  // else user chose to skip file
}
void ListedFile::delFromComputer(const fs::path computerFilename) {
    if (verbose) {
        std::cout << "del from computer: " << filename << std::endl;
    }
    if (FileOperations::remove(computerFilename)) {
        own();
    }  // else user chose to skip file
}


bool ListedFile::cmpOldHashes(const hash_t hash) const {
    for (hash_t oldHash : oldHashes) {
        if (hash == oldHash) {
            return true;
        }
    }
    return false;
}
bool ListedFile::cmpOwners(const id_t owner) const {
    for (id_t iOwner : owners) {
        if (owner == iOwner) {
            return true;
        }
    }
    return false;
}
void ListedFile::removeOwner(const id_t owner) {
    owners.erase(owner);
}

void ListedFile::addOldHash(const hash_t hash) {
    oldHashes.insert(hash);
}

void ListedFile::own() {
    owners.insert(computer->id);
}

void ListedFile::cleanIfSynced() {
    if (owners.size() == flash->getComputerCount()) {
        std::cout << filename << " is now synced across all computers" << std::endl;
        if (action == ACTION_DEL) {
            assert(flashFilename.empty());
            action = ACTION_FORGET;
        } else {
            assert(action == ACTION_SYNC);
            action = ACTION_NONE;
            delFlashFile();
        }
    }
}


void ListedFile::delFlashFile() {
    // delete file from flash, if it exists
    fs::path deletedFileOnFlash = fullFlashFilename();
    if (is_regular_file(deletedFileOnFlash)) {
        FileOperations::remove(deletedFileOnFlash);
    }
    flashFilename = "";
}


uifstream& operator>> (uifstream& is, ListedFile& a) {
    a.clean();
    
    if (!(is >> a.filename)) {
        /* eof */
        return is;
    }
    is >> a.lastWriteTime;
    is >> a.action;
    if (a.action != ACTION_DEL) {
        is >> a.hash;
        
        if (a.action == ACTION_SYNC) {
            is >> a.flashFilename;
        }
    }
    if (a.action != ACTION_NONE) {
        is >> a.owners;
        
        if (a.action == ACTION_SYNC || a.action == ACTION_DEL) {
            is >> a.oldHashes;
        }
    }
    std::cout << "";  // this somehow keeps the program from crashing
    return is;
}

uofstream& operator<< (uofstream& os, const ListedFile& a) {
    if (a.action == ACTION_FORGET) {
        // don't write anything
        return os;
    }
    os << a.filename.generic_path() << ' ' << a.lastWriteTime << ' ' << a.action;
    if (a.action != ACTION_DEL) {
        os << ' ' << a.hash;
        
        if (a.action == ACTION_SYNC) {
            os << ' ' << a.flashFilename.generic_path();
        }
    }
    if (a.action != ACTION_NONE) {
        os << ' ';
        os << a.owners;
        
        if (a.action == ACTION_SYNC || a.action == ACTION_DEL) {
            os << ' ';
            os << a.oldHashes;
        }
    }
    os << '\n';
    return os;
}

bool operator< (const ListedFile& a, const ListedFile& b) {
    return a.filename < b.filename;
}


template<typename T>
uifstream& operator>> (uifstream& is, std::set<T>& set) {
    size_t count;
    is >> count;
    for (size_t i = 0; i < count; i++) {
        T value;
        is >> value;
        set.insert(value);
    }
    return is;
}

template<typename T>
uofstream& operator<< (uofstream& os, const std::set<T>& set) {
    os << set.size();
    for (T value : set) {
        os << ' ' << value;
    }
    return os;
}
