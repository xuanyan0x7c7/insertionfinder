#include <insertionfinder/cube.hpp>
#include "./cli/parser.hpp"
using namespace InsertionFinder;


int main(int argc, char** argv) {
    Cube::init();
    CLI::parse(argc, argv);
    return 0;
}
