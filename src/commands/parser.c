#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include "parser.h"


static const char* short_options = "a:c:f:hsvV";

static const struct option long_options[] = {
    {"solve", no_argument, NULL, SHORT_COMMAND_SOLVE},
    {"verify", no_argument, NULL, SHORT_COMMAND_VERIFY},
    {"generate-algfile", no_argument, NULL, 0},
    {"help", no_argument, NULL, SHORT_COMMAND_HELP},
    {"version", no_argument, NULL, SHORT_COMMAND_VERSION},
    {"json", no_argument, NULL, 0},
    {"algfile", required_argument, NULL, SHORT_PARAMETER_ALGFILE},
    {"casefile", required_argument, NULL, SHORT_PARAMETER_CASEFILE},
    {"file", required_argument, NULL, SHORT_PARAMETER_FILE},
    {NULL, 0, NULL, 0}
};


CliParser Parse(int argc, char** argv) {
    CliParser parsed_args;
    parsed_args.command = -1;
    parsed_args.json = false;
    parsed_args.algfile_count = 0;
    parsed_args.algfile_list = (const char**)malloc(argc * sizeof(const char*));
    parsed_args.casefile_count = 0;
    parsed_args.casefile_list = (const char**)malloc(
        argc * sizeof(const char*)
    );
    parsed_args.file_count = 0;
    parsed_args.file_list = (const char**)malloc(argc * sizeof(const char*));
    while (true) {
        int option_index;
        int c = getopt_long(
            argc, argv,
            short_options, long_options,
            &option_index
        );
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                switch (option_index) {
                    case COMMAND_SOLVE:
                    case COMMAND_VERIFY:
                    case COMMAND_GENERATE_ALGFILE:
                    case COMMAND_HELP:
                    case COMMAND_VERSION:
                        parsed_args.command = option_index;
                        break;
                    case PARAMETER_JSON:
                        parsed_args.json = true;
                        break;
                    case PARAMETER_ALGFILE:
                        parsed_args.algfile_list[
                            parsed_args.algfile_count++
                        ] = optarg;
                        break;
                    case PARAMETER_CASEFILE:
                        parsed_args.casefile_list[
                            parsed_args.casefile_count++
                        ] = optarg;
                        break;
                    case PARAMETER_FILE:
                        parsed_args.file_list[
                            parsed_args.file_count++
                        ] = optarg;
                    break;
                }
                break;
            case SHORT_COMMAND_SOLVE:
                parsed_args.command = COMMAND_SOLVE;
                break;
            case SHORT_COMMAND_VERIFY:
                parsed_args.command = COMMAND_VERIFY;
                break;
            case SHORT_COMMAND_HELP:
                parsed_args.command = COMMAND_HELP;
                break;
            case SHORT_COMMAND_VERSION:
                parsed_args.command = COMMAND_VERSION;
                break;
            case SHORT_PARAMETER_ALGFILE:
                parsed_args.algfile_list[parsed_args.algfile_count++] = optarg;
                break;
            case SHORT_PARAMETER_CASEFILE:
                parsed_args.casefile_list[
                    parsed_args.casefile_count++
                ] = optarg;
                break;
            case SHORT_PARAMETER_FILE:
                parsed_args.file_list[parsed_args.file_count++] = optarg;
                break;
        }
    }
    return parsed_args;
}
