#pragma once
#include <stddef.h>

typedef struct CliParser CliParser;
struct CliParser {
    int command;
    bool json;
    size_t algfile_count;
    const char** algfile_list;
    size_t casefile_count;
    const char** casefile_list;
    size_t file_count;
    const char** file_list;
    size_t max_threads;
};

enum {
    COMMAND_SOLVE,
    COMMAND_VERIFY,
    COMMAND_GENERATE_ALGFILE,
    COMMAND_HELP,
    COMMAND_VERSION,
    PARAMETER_JSON,
    PARAMETER_ALGFILE,
    PARAMETER_CASEFILE,
    PARAMETER_FILE,
    PARAMETER_THREADS
};

enum {
    SHORT_COMMAND_SOLVE = 's',
    SHORT_COMMAND_VERIFY = 'v',
    SHORT_COMMAND_HELP = 'h',
    SHORT_COMMAND_VERSION = 'V',
    SHORT_PARAMETER_ALGFILE = 'a',
    SHORT_PARAMETER_CASEFILE = 'c',
    SHORT_PARAMETER_FILE = 'f',
    SHORT_PARAMETER_THREADS = 't'
};

CliParser parse(int argc, char** argv);
