#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <config.h>
#include "./parser.hpp"
#include "./commands.hpp"
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
        ("verify,v", "verify cube state")
    cli_options.add(command_options);

    po::options_description configuration_options("Configurations");
    configuration_options.add_options()
        ("json", "use JSON output");
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
    if (vm.count("verify")) {
        CLI::verify_cube(vm);
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
