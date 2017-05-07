#include "FileOperations.h"
#include "Interaction.h"

#include <stdio.h>
// #include <time.h>

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = boost::filesystem;


// 32-bit FNV-1a algorithm
#define FNV_PRIME 16777619
#define READ_BUFFER_SIZE 32 * 1024

hash_t FileOperations::hashFile(const fs::path& filename) {
    hash_t hash = 2166136261;
    
    FILE* file;
    try_open:
#ifdef _WIN32
    file = _wfopen(filename.native().c_str(), L"rb");
#else
    file = fopen(filename.string().c_str(), "rb");
#endif
    if (!file) {
        std::cout << "Error opening file to calculate its hash\nFilename: " << filename << std::endl;
        ask_choice: std::cout << "\tDo you want to [r]etry or [g]ive up syncing: ";
        switch (Interaction::askForChar()) {
            case 'r': goto try_open;
            case 'g': exit(ERR_HASH_OPEN_FILE);
            default: goto ask_choice;
        }
    }
    unsigned char* buffer = new unsigned char[READ_BUFFER_SIZE];
    
    // clock_t start = clock();
    size_t readSize;
    do {
        readSize = fread((void*)buffer, 1, READ_BUFFER_SIZE, file);
        for (size_t i = 0; i < readSize; i++) {
            hash ^= buffer[i];
            hash *= FNV_PRIME;
        }
    } while (readSize == READ_BUFFER_SIZE);
    // std::cout << clock() - start << std::endl;
    
    delete[] buffer;
    fclose(file);
    
    return hash;
}


bool FileOperations::copy_file(const fs::path& from, const fs::path& to, fs::copy_option option) {
    boost::system::error_code error;
    try_copy: fs::copy_file(from, to, option, error);
    if (error) {
        std::cout << "Error copying file\n\tFrom: " << from << "\n\tTo: " << to << std::endl;
        ask_choice: std::cout << "\tDo you want to [r]etry or [i]gnore the file: ";
        switch (Interaction::askForChar()) {
            case 'r': goto try_copy;
            case 'i': return false;
            default: goto ask_choice;
        }
    }
    return true;
}

bool FileOperations::remove(const fs::path& p) {
    boost::system::error_code error;
    try_remove: fs::remove(p, error);
    if (error) {
        std::cout << "Error removing file: " << p << std::endl;
        ask_choice: std::cout << "\tDo you want to [r]etry or [i]gnore the file: ";
        switch (Interaction::askForChar()) {
            case 'r': goto try_remove;
            case 'i': return false;
            default: goto ask_choice;
        }
    }
    return true;
}
