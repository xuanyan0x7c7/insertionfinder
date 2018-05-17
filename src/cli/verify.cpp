#include <iostream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm.hpp>
#include <cube.hpp>
#include "./commands.hpp"
using namespace std;
namespace pt = boost::property_tree;
using namespace InsertionFinder;


namespace {
    struct CubeCycleStatus {
        Algorithm scramble;
        Algorithm skeleton;
        bool center_rotated;
        bool center_parity;
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
        const auto [cube_placement, rotated_cube] = cube.best_placement();
        return {
            scramble, skeleton,
            cube_placement != 0,
            Cube::placement_parity(cube_placement),
            rotated_cube.has_parity(),
            rotated_cube.corner_cycles(),
            rotated_cube.edge_cycles()
        };
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError(e.what());
    }
};


template<> void CLI::verify_cube<ostream>() {
    string scramble_string;
    string skeleton_string;
    getline(cin, scramble_string);
    getline(cin, skeleton_string);
    auto status = verify(scramble_string, skeleton_string);
    cout << "Scramble: " << status.scramble << endl;
    cout << "Skeleton: " << status.skeleton << endl;
    cout << "The cube ";
    if (
        !status.center_rotated && !status.has_parity
        && status.corner_cycles == 0 && status.edge_cycles == 0
    ) {
        cout << "is already solved";
    } else {
        cout << "has ";
        if (status.center_rotated) {
            if (status.center_parity) {
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
        if (!status.center_parity && status.has_parity) {
            if (status.corner_cycles || status.edge_cycles) {
                cout << " with parity";
            } else {
                cout << "parity";
            }
        }
    }
    cout << endl;
}

template<> void CLI::verify_cube<pt::ptree>() {
    string scramble_string;
    string skeleton_string;
    getline(cin, scramble_string);
    getline(cin, skeleton_string);
    auto status = verify(scramble_string, skeleton_string);
    pt::ptree result;
    result.put("scramble", status.scramble.str());
    result.put("skeleton", status.skeleton.str());
    result.put("center_rotated", status.center_rotated);
    if (status.center_rotated) {
        result.put("center_parity", status.center_parity);
    }
    result.put("parity", status.has_parity);
    result.put("corner_cycle_num", status.corner_cycles);
    result.put("edge_cycle_num", status.edge_cycles);
    pt::write_json(cout, result);
}
