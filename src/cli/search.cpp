#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <config.h>
#include <algorithm.hpp>
#include <case.hpp>
#include <cube.hpp>
#include <finder/brute-force.hpp>
#include <finder/finder.hpp>
#include "commands.hpp"
using namespace std;
namespace po = boost::program_options;
namespace pt = boost::property_tree;
using namespace InsertionFinder;


void CLI::find_insertions(const po::variables_map& vm) {
    const vector<string> filenames = vm.count("file")
        ? vm["file"].as<vector<string>>() : vector<string>();
    const vector<string> algfilenames = vm.count("algfile")
        ? vm["algfile"].as<vector<string>>() : vector<string>();

    unordered_map<Cube, Case> map;
    for (const string& name: algfilenames) {
        ifstream fin(name, ios::in | ios::binary);
        if (fin.fail()) {
            fin = ifstream(string(ALGORITHMSDIR) + "/" + name + ".algs", ios::in | ios::binary);
            if (fin.fail()) {
                cerr << "Failed to open algorithm file " << name << endl;
                continue;
            }
        }
        size_t size;
        if (!fin.read(reinterpret_cast<char*>(&size), sizeof(size_t))) {
            cerr << "Invalid algorithm file " << name << endl;
            continue;
        }
        for (size_t i = 0; i < size; ++i) {
            Case _case;
            try {
                _case.read_from(fin);
            } catch (...) {
                cerr << "Invalid algorithm file " << name << endl;
                break;
            }
            auto node = map.find(_case.state());
            if (node == map.end()) {
                map.insert({_case.state(), move(_case)});
            } else {
                for (const Algorithm& algorithm: _case.algorithm_list()) {
                    node->second.add_algorithm(algorithm);
                }
            }
        }
    }

    vector<Case> cases;
    for (auto& node: map) {
        cases.push_back(move(node.second));
    }
    sort(cases.begin(), cases.end());

    istream* in;
    if (filenames.empty()) {
        in = &cin;
    } else {
        const string& name = filenames.front();
        ifstream* fin = new ifstream(name);
        if (fin->fail()) {
            throw CLI::CommandExecutionError("Failed to open input file" + name);
        }
        in = fin;
    }

    string line;
    getline(*in, line);
    Algorithm scramble;
    try {
        scramble = Algorithm(line);
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError("Invalid scramble " + line);
    }
    getline(*in, line);
    Algorithm skeleton;
    try {
        skeleton = Algorithm(line);
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError("Invalid skeleton " + line);
    }
    if (in != &cin) {
        delete in;
    }

    size_t max_threads = vm["jobs"].as<size_t>();
    if (max_threads == 0) {
        max_threads = static_cast<size_t>(thread::hardware_concurrency());
    }

    Cube original_cube;
    original_cube.twist(scramble);
    original_cube.twist(skeleton);
    Cube cube = original_cube.best_placement();
    bool parity = cube.has_parity();
    int corner_cycles = cube.corner_cycles();
    int edge_cycles = cube.edge_cycles();
    int placement = cube.placement();
    int placement_parity = Cube::placement_parity(placement);
    BruteForceFinder finder(scramble, skeleton, cases);

    cout << "Scramble: " << scramble << endl;
    cout << "Skeleton: " << skeleton << endl;
    cout << "The cube ";
    if (
        !placement && !parity
        && corner_cycles == 0 && edge_cycles == 0
    ) {
        cout << "is already solved";
    } else {
        cout << "has ";
        if (placement) {
            if (placement_parity) {
                cout << "parity center rotation";
            } else {
                cout << "center rotation";
            }
            if (corner_cycles || edge_cycles) {
                cout << " with ";
            }
        }
        if (corner_cycles == 1) {
            cout << "1 corner-3-cycle";
        } else if (corner_cycles > 1) {
            cout << corner_cycles << " corner-3-cycles";
        }
        if (corner_cycles && edge_cycles) {
            cout << " and ";
        }
        if (edge_cycles == 1) {
            cout << "1 edge-3-cycle";
        } else if (edge_cycles > 1) {
            cout << edge_cycles << " edge-3-cycles";
        }
        if (!placement_parity && parity) {
            if (corner_cycles || edge_cycles) {
                cout << " with parity";
            } else {
                cout << "parity";
            }
        }
    }
    cout << '.' << endl;

    finder.search(max_threads);
    auto result = finder.get_result();
    if (result.status == Finder::Status::SUCCESS) {
        const auto& solutions = finder.get_solutions();
        if (solutions.empty()) {
            cout << "No solution found." << endl;
        }
        for (size_t index = 0; index < solutions.size(); ++index) {
            const auto& solution = solutions[index];
            cout << endl << "Solution #" << index + 1 << endl;
            for (size_t i = 0; i < solution.insertions.size() - 1; ++i) {
                const auto& insertion = solution.insertions[i];
                const Algorithm& skeleton = insertion.skeleton;
                size_t insert_place = insertion.insert_place;
                if (insert_place > 0) {
                    skeleton.print(cout, 0, insert_place);
                    cout << ' ';
                }
                cout << "[@" << i + 1 << ']';
                if (insert_place < skeleton.length()) {
                    cout << ' ';
                    skeleton.print(cout, insert_place, skeleton.length());
                }
                cout << endl;
                cout << "Insert at @" << i + 1 << ": " << *insertion.insertion << endl;
            }
            cout
                << "Total moves: " << finder.get_fewest_moves() << ", "
                << solution.cancellation << " move" << (solution.cancellation == 1 ? "" : "s")
                << " cancelled." << endl
                << "Full solution: " << solution.insertions.back().skeleton << endl;
        }
    } else if (result.status == Finder::Status::FAILURE_PARITY_ALGORITHMS_NEEDED) {
        cout << "Parity algorithms needed." << endl;
    } else if (result.status == Finder::Status::FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED) {
        cout << "Corner cycle algorithms needed." << endl;
    } else if (result.status == Finder::Status::FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED) {
        cout << "Edge cycle algorithms needed." << endl;
    } else if (result.status == Finder::Status::FAILURE_CENTER_ALGORITHMS_NEEDED) {
        cout << "Center algorithms needed." << endl;
    }
    cout << "Time usage: " << fixed << setprecision(3);
    if (result.duration < 1000) {
        cout << result.duration << " nanoseconds." << endl;
    } else if (result.duration < 1000000) {
        cout << result.duration / 1e3 << " microseconds." << endl;
    } else if (result.duration < 1000000000) {
        cout << result.duration / 1e6 << " milliseconds." << endl;
    } else if (result.duration < 60 * static_cast<int64_t>(1000000000)) {
        cout << result.duration / 1e9 << " seconds." << endl;
    } else if (result.duration < 60 * 60 * static_cast<int64_t>(1000000000)) {
        int64_t duration = (result.duration + 500000) / 1000000;
        cout << duration / (60 * 1000)
            << right << setfill('0')
            << ':' << setw(2) << duration / 1000 % 60
             << '.'<< setw(3) << duration % 1000 << endl;
    } else {
        int64_t duration = (result.duration + 500000) / 1000000;
        cout << duration / (60 * 60 * 1000)
            << right << setfill('0')
            << ':' << setw(2) << duration / (60 * 1000) % 60
            << ':' << setw(2) << duration / 1000 % 60
            << ':' << setw(3) << duration % 1000 << '.' << endl;
    }
}
