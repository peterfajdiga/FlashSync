DIR=$(dirname $0)
g++ -std=c++14 $DIR/FlashSync.cpp $DIR/Config.cpp $DIR/ConfigComputer.cpp $DIR/ConfigFlash.cpp $DIR/FileOperations.cpp $DIR/Interaction.cpp $DIR/ListedFile.cpp $DIR/Synchronization.cpp -o $DIR/FlashSync -lboost_system -lboost_filesystem -lboost_random
