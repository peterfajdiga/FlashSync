mkdir build
g++ -std=c++14 source\FlashSync.cpp source\Config.cpp source\ConfigComputer.cpp source\ConfigFlash.cpp source\FileOperations.cpp source\Interaction.cpp source\ListedFile.cpp source\Synchronization.cpp source\uifstream.cpp source\uofstream.cpp -o build\FlashSync.exe -lboost_system -lboost_filesystem -lboost_locale -lboost_random
