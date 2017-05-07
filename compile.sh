DIR=$(dirname $0)
mkdir $DIR/build
g++ -std=c++14 $DIR/source/FlashSync.cpp $DIR/source/Config.cpp $DIR/source/ConfigComputer.cpp $DIR/source/ConfigFlash.cpp $DIR/source/FileOperations.cpp $DIR/source/Interaction.cpp $DIR/source/ListedFile.cpp $DIR/source/Synchronization.cpp -o $DIR/build/FlashSync -lboost_system -lboost_filesystem -lboost_random
