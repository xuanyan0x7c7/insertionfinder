#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <algorithm.hpp>
#include <cube.hpp>
#include "commands.hpp"
#include "univalue/univalue.h"
using namespace std;
namespace po = boost::program_options;
using namespace InsertionFinder;


namespace {
    struct CubeCycleStatus {
        Algorithm scramble;
        Algorithm skeleton;
        bool has_parity;
        int corner_cycles;
        int edge_cycles;
        int center_cycles;
    };

    CubeCycleStatus
    verify(const string& scramble_string, const string& skeleton_string) try {
        Algorithm scramble(scramble_string);
        Algorithm skeleton(skeleton_string);
        Cube cube;
        cube.twist(scramble);
        cube.twist(skeleton);
        const Cube placed_cube = cube.best_placement();
        return {
            scramble, skeleton,
            placed_cube.has_parity(),
            placed_cube.corner_cycles(),
            placed_cube.edge_cycles(),
            Cube::center_cycles[placed_cube.placement()]
        };
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError(e.what());
    }
};


namespace {
    template<class T> void print_result(const CubeCycleStatus& status);

    template<> void print_result<ostream>(const CubeCycleStatus& status) {
        cout << "Scramble: " << status.scramble << endl;
        cout << "Skeleton: " << status.skeleton << endl;
        cout << "The cube ";
        if (
            !status.has_parity
            && status.corner_cycles == 0 && status.edge_cycles == 0
            && status.center_cycles == 0
        ) {
            cout << "is already solved";
        } else {
            cout << "has ";
            if (status.center_cycles) {
                if (status.center_cycles > 1) {
                    cout << "parity center rotation";
                } else {
                    cout << "center rotation";
                }
                if (status.corner_cycles || status.edge_cycles) {
                    cout << " with ";
                }
            }
            if (status.corner_cycles == 1) {
                cout << "1 corner-3-cycle";
            } else if (status.corner_cycles > 1) {
                cout << status.corner_cycles << " corner-3-cycles";
            }
            if (status.corner_cycles && status.edge_cycles) {
                cout << " and ";
            }
            if (status.edge_cycles == 1) {
                cout << "1 edge-3-cycle";
            } else if (status.edge_cycles > 1) {
                cout << status.edge_cycles << " edge-3-cycles";
            }
            if (status.center_cycles <= 1 && status.has_parity) {
                if (status.corner_cycles || status.edge_cycles) {
                    cout << " with parity";
                } else {
                    cout << "parity";
                }
            }
        }
        cout << '.' << endl;
    }

    template<> void print_result<UniValue>(const CubeCycleStatus& status) {
        UniValue map(UniValue::VOBJ);
        map.pushKV("scramble", status.scramble.str());
        map.pushKV("skeleton", status.skeleton.str());
        map.pushKV("parity", status.has_parity);
        map.pushKV("corner_cycles", status.corner_cycles);
        map.pushKV("edge_cycles", status.edge_cycles);
        map.pushKV("center_cycles", status.center_cycles);
        cout << map.write() << flush;
    }
};


void CLI::verify_cube(const po::variables_map& vm) {
    string scramble_string;
    string skeleton_string;
    getline(cin, scramble_string);
    getline(cin, skeleton_string);
    auto status = verify(scramble_string, skeleton_string);
    if (vm.count("json")) {
        ::print_result<UniValue>(status);
    } else {
        ::print_result<ostream>(status);
    }
}
