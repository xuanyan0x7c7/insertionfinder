#pragma once
#include <stddef.h>

typedef struct CliParser CliParser;
struct CliParser {
    int command;
    size_t file_count;
    const char** file_list;
    size_t algfile_count;
    const char** algfile_list;
};

enum {
    COMMAND_SOLVE,
    COMMAND_VERIFY,
    COMMAND_GENERATE_ALGFILE,
    COMMAND_HELP,
    COMMAND_VERSION,
    PARAMETER_FILE,
    PARAMETER_ALGFILE
};

enum {
    SHORT_COMMAND_SOLVE = 's',
    SHORT_COMMAND_VERIFY = 'v',
    SHORT_COMMAND_HELP = 'h',
    SHORT_COMMAND_VERSION = 'V',
    SHORT_PARAMETER_FILE = 'f',
    SHORT_PARAMETER_ALGFILE = 'a'
};

CliParser Parse(int argc, char** argv);
