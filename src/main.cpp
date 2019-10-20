#include "./cli/parser.hpp"


int main(int argc, char** argv) {
    InsertionFinder::CLI::parse(argc, argv);
    return 0;
}
