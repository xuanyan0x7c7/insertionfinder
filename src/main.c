#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include "commands/parser.h"
#include "commands/verify.h"


int main(int argc, char** argv) {
    CliParser parsed_args = Parse(argc, argv);
    switch (parsed_args.command) {
        case COMMAND_SOLVE:
            puts("Under development");
            break;
        case COMMAND_VERIFY:
            if (!Verify(&parsed_args)) {
                exit(EXIT_FAILURE);
            }
            break;
        case COMMAND_GENERATE_ALGFILE:
            puts("Under development");
            break;
        case COMMAND_HELP:
            puts(PACKAGE_STRING);
            break;
        case COMMAND_VERSION:
            puts(PACKAGE_STRING);
            break;
    }
    exit(EXIT_SUCCESS);
}
