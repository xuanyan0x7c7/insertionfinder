#pragma once
#include <stddef.h>

typedef struct CliParser CliParser;
struct CliParser {
    int command;
    size_t file_count;
    const char** file_list;
};

enum {
    COMMAND_SOLVE,
    COMMAND_VERIFY,
    COMMAND_GENERATE_ALGFILE,
    COMMAND_HELP,
    COMMAND_VERSION,
    PARAMETER_FILE,
    SHORT_COMMAND_SOLVE = 's',
    SHORT_COMMAND_VERIFY = 'v',
    SHORT_COMMAND_HELP = 'h',
    SHORT_COMMAND_VERSION = 'V',
    SHORT_PARAMETER_FILE = 'f'
};

CliParser Parse(int argc, char** argv);
