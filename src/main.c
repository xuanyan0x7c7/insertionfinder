#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include "cube/cube.h"
#include "commands/commands.h"
#include "commands/parser.h"


typedef bool Executor(const CliParser* parsed_args);

bool Version(const CliParser* parsed_args) {
    puts(PACKAGE_STRING);
    return true;
}


int main(int argc, char** argv) {
    CubeInitialize();

    Executor* executor = NULL;
    CliParser parsed_args = Parse(argc, argv);
    switch (parsed_args.command) {
        case COMMAND_SOLVE:
            executor = Solve;
            break;
        case COMMAND_VERIFY:
            executor = Verify;
            break;
        case COMMAND_GENERATE_ALGFILE:
            executor = GenerateAlgfiles;
            break;
        case COMMAND_HELP:
        case COMMAND_VERSION:
            executor = Version;
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
