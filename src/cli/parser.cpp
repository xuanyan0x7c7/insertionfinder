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
using namespace std;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
using namespace InsertionFinder;


namespace {
    const char* version_string = PACKAGE_NAME " v" VERSION;
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
        ("verify,v", "verify cube state")
        ("generate,g", "generate algorithm files");
    cli_options.add(command_options);

    po::options_description configuration_options("Configurations");
    configuration_options.add_options()
        ("algfile,a", po::value<vector<string>>(), "algorithm file")
        (
            "algs-dir",
            po::value<string>()->default_value(ALGORITHMSDIR),
            "algorithms directory"
        )
        ("all-algs", "all algorithms")
        ("all-extra-algs", "all extra algorithms")
        ("file,f", po::value<vector<string>>(), "input file")
        ("optimal,o", "search for optimal solutions")
        ("target", po::value<size_t>(), "search target")
        (
            "greedy-threshold",
            po::value<size_t>()->default_value(2),
            "suboptimal moves tolerance"
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
                ->implicit_value(thread::hardware_concurrency()),
            "multiple threads"
        )
        ("json", "use JSON output")
        ("verbose", "verbose");
    cli_options.add(configuration_options);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cli_options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << version_string << endl << cli_options << endl;
        return;
    }
    if (vm.count("version")) {
        cout << version_string << endl;
        return;
    }
    if (vm.count("solve")) {
        CLI::find_insertions(vm);
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
    cout << version_string << endl << cli_options << endl;
} catch (const po::error& e) {
    cerr << e.what() << endl;
    exit(EXIT_FAILURE);
} catch (const CLI::CommandExecutionError& e) {
    cerr << e.what() << endl;
    exit(EXIT_FAILURE);
}
