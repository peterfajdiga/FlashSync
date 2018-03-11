#include "Synchronization.h"

#include "ConfigComputer.h"
#include "ConfigFlash.h"
#include "FileOperations.h"
#include "global.h"
#include "Interaction.h"
#include "ListedFile.h"
#include "uifstream.h"
#include "uofstream.h"

#include <iostream>
#include <vector>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


namespace {
    std::vector<fs::path> computerFiles;
    std::vector<fs::path>::iterator iLocal;
    std::vector<ListedFile> listedFiles;
    std::vector<ListedFile>::iterator iListed;
    uofstream listFileDest;
    
    void firstSync();
    void bothFiles(fs::path computerFilename);
    void computerFileOnly(fs::path computerFilename);
    void listedFileOnly();
    bool hasNextComputerFile();
    bool hasNextListedFile();
    
    void conflict(fs::path filename, hash_t hash, bool deleted = false);
    void warnTimestampNotGreater(fs::path filename, bool deleted = false);
    void warnTimestampNotLesser(fs::path filename);
    
    template<typename T>
    void vectorOrderedInsert(std::vector<T>& v, const T& element);
}
    
extern ConfigComputer* computer;
extern ConfigFlash* flash;
extern bool compareTimestamps;


void Synchronization::sync() {
    fs::path listFilename = flash->dir / "list";
    if (!fs::is_regular_file(listFilename)) {
        firstSync();
        return;
    }
    
    // load computer files
    for (fs::recursive_directory_iterator i(computer->syncDir), end; i != end; i++) {
        vectorOrderedInsert<fs::path>(computerFiles, *i);
    }
    iLocal = computerFiles.begin();
    
    // load listed files
    fs::path oldListFilename = listFilename;
    oldListFilename += "~";
    rename(listFilename, oldListFilename);
    ListedFile file;
    for (uifstream listFileSrc(oldListFilename); listFileSrc >> file;) {
        vectorOrderedInsert<ListedFile>(listedFiles, file);
    }
    iListed = listedFiles.begin();
    
    listFileDest.open(listFilename);
    
    while (hasNextListedFile() || hasNextComputerFile()) {
        if (!hasNextComputerFile()) {
            listedFileOnly();               // sync file and advance listedFiles
            continue;
        }
        const fs::path filename = *iLocal;
        if (!fs::is_regular_file(filename)) {
            iLocal++;                       // advance computerFiles
            continue;
        }
        if (!hasNextListedFile()) {
            computerFileOnly(filename);     // sync file and advance computerFiles
            continue;
        }
        
        const fs::path filenameRelative = relative(filename, computer->syncDir);
        int cmp = filenameRelative.compare(iListed->filename);
        if (cmp == 0) {
            bothFiles(filename);            // sync file and advance both iterators
        } else if (cmp < 0) {
            computerFileOnly(filename);     // sync file and advance computerFiles
        } else {
            listedFileOnly();               // sync file and advance listedFiles
        }
    }
}

void Synchronization::removeComputer(const id_t computer_id) {
    fs::path listFilename = flash->dir / "list";
    if (!fs::is_regular_file(listFilename)) {
        // nothing to do
        return;
    }
    fs::path oldListFilename = listFilename;
    oldListFilename += "~";
    rename(listFilename, oldListFilename);
    
    ListedFile file;
    listFileDest.open(listFilename);
    for (uifstream listFileSrc(oldListFilename); listFileSrc >> file;) {
        file.removeOwner(computer_id);
        listFileDest << file;
    }
}



namespace {
    void firstSync() {
        std::cout << "Creating list file.\n";
        
        uofstream listFileDest(flash->dir / "list");
        for (fs::recursive_directory_iterator i(computer->syncDir), end; i != end; i++) {
            const fs::path file = *i;
            if (!fs::is_regular_file(file)) {
                continue;
            }
            // TODO: use ListedFile instead of bottom line
            listFileDest << relative(file, computer->syncDir).generic_path() << ' ' << fs::last_write_time(file) << ' ' << 0 << ' ' << FileOperations::hashFile(file) << std::endl;
        }
        listFileDest.close();
        if (listFileDest.fail()) {
            std::cerr << "Error creating a list file in " << flash->dir << std::endl;
            exit(ERR_CONFIG_SAVE);
        }
    }

    void bothFiles(fs::path computerFilename) {
        hash_t hash = FileOperations::hashFile(computerFilename);
        time_t lastWriteTime = fs::last_write_time(computerFilename);
        switch (iListed->action) {
            case ACTION_NONE: {
                if ((compareTimestamps && lastWriteTime == iListed->lastWriteTime) || hash == iListed->hash) {
                    // the files are identical
                    break;
                }
                // computer file is newer than flash
                // assert(lastWriteTime > iListed->lastWriteTime);
                if (lastWriteTime <= iListed->lastWriteTime) {
                    std::cout << "File changed" << std::endl;
                    warnTimestampNotGreater(computerFilename);
                }
                iListed->syncToFlash(hash, computerFilename);
                break;
            }
            case ACTION_SYNC: {
                if ((compareTimestamps && lastWriteTime == iListed->lastWriteTime) || hash == iListed->hash) {
                    // the files are identical, nothing to do
                    iListed->own();
                    break;
                }
                if (iListed->cmpOwners(computer->id)) {
                    // computer file is newer than flash, since we already own the flash version
                    // assert(lastWriteTime > iListed->lastWriteTime);
                    if (lastWriteTime <= iListed->lastWriteTime) {
                        std::cout << "File changed" << std::endl;
                        warnTimestampNotGreater(computerFilename);
                    }
                    iListed->syncToFlash(hash, computerFilename);
                    break;
                }
                if (iListed->cmpOldHashes(hash)) {
                    // computer file is older than flash
                    if (lastWriteTime >= iListed->lastWriteTime) {
                        std::cout << "Remote file changed" << std::endl;
                        warnTimestampNotLesser(computerFilename);
                    }
                    // sync the file
                    iListed->syncToComputer(computerFilename);
                    break;
                }
                conflict(computerFilename, hash);
                break;
            }
            case ACTION_DEL: {
                if (iListed->cmpOwners(computer->id)) {
                    // file was restored
                    // assert(lastWriteTime > iListed->lastWriteTime);
                    if (lastWriteTime <= iListed->lastWriteTime) {
                        std::cout << "File restored" << std::endl;
                        warnTimestampNotGreater(computerFilename);
                    }
                    iListed->syncToFlash(hash, computerFilename);
                    break;
                }
                if (iListed->cmpOldHashes(hash)) {
                    // safe to delete
                    iListed->delFromComputer(computerFilename);
                    break;
                }
                // File was modified. Conflict!
                conflict(computerFilename, hash, true);
            }
        }
        iLocal++;
        iListed->cleanIfSynced();
        listFileDest << *iListed;
        iListed++;
    }

    void computerFileOnly(fs::path computerFilename) {
        ListedFile newListedFile;
        newListedFile.addToFlash(computerFilename);
        newListedFile.cleanIfSynced();
        listFileDest << newListedFile;
        
        iLocal++;
    }

    void listedFileOnly() {
        if (iListed->action == ACTION_SYNC && !iListed->cmpOwners(computer->id)) {
            // file not yet added to computer, add it
            iListed->addToComputer();
        } else {
            // deleted file, mark it
            iListed->delFromFlash();
        }
        iListed->cleanIfSynced();
        listFileDest << *iListed;
        iListed++;
    }


    bool hasNextComputerFile() {
        return iLocal < computerFiles.end();
    }

    bool hasNextListedFile() {
        return iListed < listedFiles.end();
    }


    void conflict(fs::path computerFilename, hash_t hash, bool deleted) {
        std::cout << "\nConflict!" <<
            "\n\tComputer file: " << computerFilename <<
            "\n\tFlash file: " << (deleted ? "*deleted*" : iListed->fullFlashFilename()) <<
            std::endl;
        
        resolveConflict:
        std::cout << "\n\tDo you want to keep the [f]lash state or your [c]omputer state?\n\tIf you [s]kip the file, you will end up in an unsynced state: ";
        switch (Interaction::askForChar()) {
            case 'f': {
                std::cout << "\n\tAre you sure you want to keep the flash state? This will delete your computer file forever!\n\tType [y] to continue or type anything else to choose differently: ";
                if (Interaction::askForChar() != 'y') {
                    goto resolveConflict;
                }
                iListed->addOldHash(hash); // Mark the computer file as obsolete
                if (deleted) {
                    iListed->delFromComputer(computerFilename);
                } else {
                    iListed->syncToComputer(computerFilename);
                }
                break;
            }
            case 'c': {
                std::cout << "\n\tAre you sure you want to keep the computer file? It will replace the flash file and be synced to all computers!\n\tType [y] to continue or type anything else to choose differently: ";
                if (Interaction::askForChar() != 'y') {
                    goto resolveConflict;
                }
                iListed->syncToFlash(hash, computerFilename);
                break;
            }
            case 's': {
                std::cout << "Skipped:        " << iListed->filename << std::endl;
                break;
            }
            default: {
                goto resolveConflict;
            }
        }
    }
    
    void warnTimestampNotGreater(fs::path computerFilename, bool deleted) {
        std::cout << "Warning: Timestamp of modified file not larger than that of stored." <<
            "\n\tComputer file: " << computerFilename <<
            "\n\tFlash file: " << (deleted ? "*deleted*" : iListed->fullFlashFilename()) <<
            std::endl;
        
        getConfirmation:
        std::cout << "\n\rCheck files and type [y] to continue: ";
        switch (Interaction::askForChar()) {
            case 'y': return;
            default:  goto getConfirmation;
        }
    }
    
    void warnTimestampNotLesser(fs::path computerFilename) {
        std::cout << "Warning: Timestamp of computer's file not lesser than that of stored (modified)." <<
            "\n\tComputer file: " << computerFilename <<
            "\n\tFlash file: " << iListed->fullFlashFilename() <<
            std::endl;
        
        getConfirmation:
        std::cout << "\n\rCheck files and type [y] to continue: ";
        switch (Interaction::askForChar()) {
            case 'y': return;
            default:  goto getConfirmation;
        }
    }


    template<typename T>
    void vectorOrderedInsert(std::vector<T>& v, const T& element) {
        typename std::vector<T>::reverse_iterator position = v.rbegin();
        while (position < v.rend() && element < *position) {
            position++;
        }
        v.insert(position.base(), element);
    }
}
