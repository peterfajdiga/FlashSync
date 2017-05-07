#include "Interaction.h"

#include "global.h"

#include <iostream>


namespace {
    const char* USAGE_STR = "Usage: FlashSync [options] <flash drive location>\n"
                            "       \t\t(use above line to sync)\n"
                            "   or: FlashSync id\n"
                            "       \t\t(use above line to print computer id)\n"
                            "   or: FlashSync <command> [<argument>] <flash drive location>\n";
}


char Interaction::askForChar() {
    char choice;
    std::cin >> choice;
    return tolower(choice);
}

void Interaction::exitErrArgs() {
    std::cerr << USAGE_STR << "\nTry 'FlashSync -h' for more information.\n";
    exit(ERR_ARGS);
}

void Interaction::exitErrCmd(const char* command) {
    std::cerr << "Invalid command: " << command << "\n\n";
    exitErrArgs();
}

void Interaction::printHelp() {
    std::cout << USAGE_STR <<
        "\nCommands:\n"
        "  id        \tPrint flash id\n"
        "  syncdir   \tPrint this computer's sync dir for the selected flash drive\n"
        "  setsyncdir <new sync dir>\n"
        "            \tChange the sync dir\n"
        "  unregister\tUnregister this computer from the selected flash drive\n"
        "  unregister <computer id>\n"
        "            \tUnregister specified computer from the selected flash drive\n"
        "  computers \tPrint all computer ids registered on the selected flash drive\n"
        "\nOptions:\n"
        "  -s        \tSilent mode\n"
        "  -t        \tCompare timestamps instead of computing checksums for all files"
        "  -h        \tPrint this message\n";
}
