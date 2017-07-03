#pragma once
#include <stdbool.h>
#include "parser.h"

bool Solve(const CliParser* parsed_args);
bool Verify(const CliParser* parsed_args);
void GenerateAlgfiles(const CliParser* parsed_args);
