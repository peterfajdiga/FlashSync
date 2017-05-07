#include "ConfigComputer.h"
#include "ConfigFlash.h"
#include "global.h"
#include "Interaction.h"
#include "Synchronization.h"

#include <assert.h>
#include <getopt.h>

#include <iostream>

namespace fs = boost::filesystem;


ConfigComputer* computer;
ConfigFlash* flash;
bool verbose = true;
bool compareTimestamps = false;

fs::path getComputerLocation() {
    const char* homedir;
#ifdef _WIN32
    homedir = getenv("APPDATA");
#else
    homedir = getenv("HOME");
#endif
    if (homedir == NULL) {
        std::cerr << "HOME environment variable not set.\n";
        exit(ERR_NO_HOME_DIR);
    }
    
    return fs::path(homedir) / ".FlashSync";
}

int main(int argc, char* argv[]) {
    while (true) {
        switch (getopt(argc, argv, "sth")) {
            case 's': verbose = false; break;
            case 't': compareTimestamps = true; break;
            case 'h': Interaction::printHelp(); return 0;
            case '?': Interaction::exitErrArgs();
            case -1: goto exit_getopt_loop;  // break loop
            default: assert(false); return ERR_ARGS;
        }
    }
    exit_getopt_loop:
    
    argc -= optind;
    argv += optind;
    
    switch (argc) {
        case 1: {
            if (strcmp(argv[0], "id") == 0) {
                computer = new ConfigComputer(getComputerLocation());
                std::cout << computer->id << std::endl;
                
            } else {
                fs::path flashPath(argv[0]);
                if (!fs::is_directory(flashPath)) {
                    std::cerr << flashPath << " is not a directory.\n\n";
                    Interaction::exitErrArgs();
                }
                flash = new ConfigFlash(flashPath / "FlashSync");
                computer = new ConfigComputer(getComputerLocation());
                flash->registerComputer(computer->id);
                computer->loadPathFile(flash->id);

                Synchronization::sync();
            }
            
            break;
        }
        
        case 2: {
            if (strcmp(argv[0], "id") == 0) {
                flash = new ConfigFlash(fs::path(argv[1]) / "FlashSync");
                std::cout << flash->id << std::endl;
                
            } else if (strcmp(argv[0], "syncdir") == 0) {
                flash = new ConfigFlash(fs::path(argv[1]) / "FlashSync");
                computer = new ConfigComputer(getComputerLocation());
                computer->loadPathFile(flash->id);
                std::cout << computer->syncDir << std::endl;
                
            } else if (strcmp(argv[0], "unregister") == 0) {
                flash = new ConfigFlash(fs::path(argv[1]) / "FlashSync");
                computer = new ConfigComputer(getComputerLocation());
                flash->unregisterComputer(computer->id);
                computer->unregister(flash->id);
                Synchronization::removeComputer(computer->id);
                
            } else if (strcmp(argv[0], "computers") == 0) {
                flash = new ConfigFlash(fs::path(argv[1]) / "FlashSync");
                flash->printComputers();
                
            } else {
                Interaction::exitErrCmd(argv[0]);
            }
            
            break;
        }
        
        case 3: {
            if (strcmp(argv[0], "setsyncdir") == 0) {
                flash = new ConfigFlash(fs::path(argv[2]) / "FlashSync");
                computer = new ConfigComputer(getComputerLocation());
                computer->setSyncDir(flash->id, argv[1]);
                
            } else if (strcmp(argv[0], "unregister") == 0) {
                flash = new ConfigFlash(fs::path(argv[2]) / "FlashSync");
                const id_t computerToUnregister = atoi(argv[1]);
                flash->unregisterComputer(computerToUnregister);
                Synchronization::removeComputer(computerToUnregister);
                
            } else {
                Interaction::exitErrCmd(argv[0]);
            }
            
            break;
        }
        
        default: Interaction::exitErrArgs();
    }
    
    
    // cleanup
    delete computer;
    delete flash;
    
    return 0;
}
