#pragma once
#include <stdbool.h>
#include "parser.h"

bool solve(const CliParser* parsed_args);
bool verify(const CliParser* parsed_args);
bool generate_alg_files(const CliParser* parsed_args);
