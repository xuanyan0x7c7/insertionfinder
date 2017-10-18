#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include "init.h"
#include "commands/commands.h"
#include "commands/parser.h"


typedef bool Executor(const CliParser* parsed_args);

static bool version(const CliParser* parsed_args) {
    puts("Insertion Finder " VERSION);
    return true;
}


int main(int argc, char** argv) {
    init();

    Executor* executor = NULL;
    CliParser parsed_args = parse(argc, argv);
    switch (parsed_args.command) {
        case COMMAND_SOLVE:
            executor = solve;
            break;
        case COMMAND_VERIFY:
            executor = verify;
            break;
        case COMMAND_GENERATE_ALGFILE:
            executor = generate_alg_files;
            break;
        case COMMAND_HELP:
        case COMMAND_VERSION:
            executor = version;
            break;
        default:
            executor = NULL;
            break;
    }

    if (executor && !executor(&parsed_args)) {
        exit(EXIT_FAILURE);
    } else {
        exit(EXIT_SUCCESS);
    }
}
