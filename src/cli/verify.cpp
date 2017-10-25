#include <iostream>
#include <istream>
#include <string>
#include <algorithm.hpp>
#include <cube.hpp>
#include "./commands.hpp"
using namespace std;
using namespace InsertionFinder;


namespace {
    struct CubeCycleStatus {
        bool has_parity;
        int corner_cycles;
        int edge_cycles;
    };

    CubeCycleStatus
    verify(const string& scramble_string, const string& skeleton_string) try {
        Algorithm scramble(scramble_string);
        Algorithm skeleton(skeleton_string);
        Cube cube;
        cube.twist(scramble);
        cube.twist(skeleton);
        return {cube.has_parity(), cube.corner_cycles(), cube.edge_cycles()};
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError(e.what());
    }
};


template<> void CLI::verify_cube<istream>() {
    string scramble_string;
    string skeleton_string;
    getline(cin, scramble_string);
    getline(cin, skeleton_string);
    auto status = verify(scramble_string, skeleton_string);
    cout << "The cube ";
    if (!status.has_parity && status.corner_cycles == 0 && status.edge_cycles == 0) {
        cout << "is already solved";
    } else {
        cout << "has ";
        if (status.has_parity) {
            cout << "parity with ";
            if (status.corner_cycles == 0 && status.edge_cycles == 0) {
                cout << "no additional cycles";
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
    }
    cout << endl;
}
