#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/program_options.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include "commands.hpp"
using namespace std;
namespace po = boost::program_options;
using namespace InsertionFinder;


void CLI::generate_algorithms(const po::variables_map& vm) {
    const vector<string> filenames = vm.count("file")
        ? vm["file"].as<vector<string>>() : vector<string>();
    const vector<string> algfilenames = vm.count("algfile")
        ? vm["algfile"].as<vector<string>>() : vector<string>();

    unordered_map<Cube, Case> map;

    ostream* out;
    if (algfilenames.empty()) {
        out = &cout;
    } else {
        const string& name = algfilenames.front();
        ofstream* fout = new ofstream(name, ios::out | ios::binary);
        if (fout->fail()) {
            throw CLI::CommandExecutionError("Failed to open output file" + name);
        }
        out = fout;
    }

    for (const string& name: filenames) {
        ifstream fin(name);
        if (fin.fail()) {
            cerr << "Failed to open file " << name << endl;
            continue;
        }
        while (!fin.eof()) {
            string line;
            getline(fin, line);
            Algorithm algorithm;
            try {
                algorithm = Algorithm(line);
            } catch (const AlgorithmError& e) {
                cerr << "Invalid algorithm: " << e.what() << endl;
                continue;
            }
            algorithm.normalize();
            algorithm.detect_rotation();
            Cube cube;
            cube.twist(algorithm);
            if (cube.mask() == 0) {
                continue;
            }

            auto node = map.find(cube);
            if (node != map.end() && node->second.contains_algorithm(algorithm)) {
                continue;
            }
            const auto isomorphism_list = algorithm.generate_isomorphisms();
            for (const Algorithm& algorithm: isomorphism_list) {
                Cube cube;
                cube.twist(algorithm);
                auto node = map.find(cube);
                if (node != map.end()) {
                    node->second.add_algorithm(algorithm);
                } else {
                    Case _case(cube);
                    _case.add_algorithm(algorithm);
                    map.insert({cube, move(_case)});
                }
            }
        }
    }

    const size_t size = map.size();
    vector<Case> cases;
    cases.reserve(size);
    for (auto& node: map) {
        cases.push_back(move(node.second));
    }
    sort(cases.begin(), cases.end());
    out->write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    for (auto& _case: cases) {
        _case.sort_algorithms();
        _case.save_to(*out);
    }

    if (out != &cout) {
        delete out;
    }
}
