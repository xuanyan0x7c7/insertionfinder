#include "./cli/parser.hpp"
using namespace InsertionFinder;


int main(int argc, char** argv) {
    CLI::parse(argc, argv);
    return 0;
}
