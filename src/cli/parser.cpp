#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <config.h>
#include "parser.hpp"
#include "commands.hpp"
using std::size_t;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace CLI = InsertionFinder::CLI;


namespace {
    constexpr const char* version_string = PACKAGE_NAME " v" VERSION;
};


void CLI::parse(int argc, char** argv) try {
    po::options_description cli_options;

    po::options_description general_options("General options");
    general_options.add_options()
        ("version", "print version string")
        ("help,h", "produce help message");
    cli_options.add(general_options);

    po::options_description command_options("Commands");
    command_options.add_options()
        ("solve,s", "find insertions!")
        ("improve,i", "find improvements!")
        ("verify,v", "verify cube state")
        ("generate,g", "generate algorithm files");
    cli_options.add(command_options);

    po::options_description configuration_options("Configurations");
    configuration_options.add_options()
        ("algfile,a", po::value<std::vector<std::string>>(), "algorithm file")
        (
            "algs-dir",
            po::value<std::string>()->default_value(ALGORITHMSDIR),
            "algorithms directory"
        )
        ("all-algs", "all algorithms")
        ("all-extra-algs", "all extra algorithms")
        ("file,f", po::value<std::vector<std::string>>(), "input file")
        ("optimal,o", "search for optimal solutions")
        ("target", po::value<size_t>(), "search target")
        ("enable-replacement", "enable replacement")
        (
            "greedy-threshold",
            po::value<size_t>()->default_value(2),
            "suboptimal moves tolerance"
        )
        (
            "replacement-threshold",
            po::value<size_t>()->default_value(0),
            "tolerance to insert an algorithm remains number of insertions"
        )
        (
            "parity",
            po::value<double>()->default_value(1.5),
            "count parity as 1/1.5 cycles"
        )
        (
            "jobs,j",
            po::value<size_t>()
                ->default_value(1)
                ->implicit_value(std::thread::hardware_concurrency()),
            "multiple threads"
        )
        ("symmetrics-only", "generate only symmetric algorithms")
        ("json", "use JSON output")
        ("verbose", "verbose");
    cli_options.add(configuration_options);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cli_options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << version_string << std::endl << cli_options << std::endl;
        return;
    }
    if (vm.count("version")) {
        std::cout << version_string << std::endl;
        return;
    }
    if (vm.count("solve")) {
        CLI::find_insertions(vm);
        return;
    }
    if (vm.count("improve")) {
        CLI::find_improvements(vm);
        return;
    }
    if (vm.count("verify")) {
        CLI::verify_cube(vm);
        return;
    }
    if (vm.count("generate")) {
        CLI::generate_algorithms(vm);
        return;
    }
    std::cout << version_string << std::endl << cli_options << std::endl;
} catch (const po::error& e) {
    std::cerr << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
} catch (const CLI::CommandExecutionError& e) {
    std::cerr << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
}
