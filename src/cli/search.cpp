#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/program_options.hpp>
#include <insertionfinder/fallbacks/filesystem.hpp>
#include <config.h>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/brute-force.hpp>
#include <insertionfinder/finder/greedy.hpp>
#include <insertionfinder/finder/finder.hpp>
#include "commands.hpp"
#include "univalue/univalue.h"
using namespace std;
namespace fs = std::filesystem;
namespace po = boost::program_options;
using namespace InsertionFinder;


namespace {
    struct CycleStatus {
        bool parity;
        int corner_cycles;
        int edge_cycles;
        int center_cycles;
    };

    struct Printer {
        virtual void print_case_information(
            const Algorithm& scramble, const Algorithm& skeleton,
            const CycleStatus& status
        ) {}
        virtual void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            const CycleStatus& status,
            const Finder& finder, const Finder::Result& result
        ) = 0;
        virtual ~Printer() = default;
    };

    struct StandardPrinter: Printer {
        void print_case_information(
            const Algorithm& scramble, const Algorithm& skeleton,
            const CycleStatus& status
        ) override {
            cout << "Scramble: " << scramble << endl;
            cout << "Skeleton: " << skeleton << endl;
            cout << "The cube ";
            if (!status.parity && status.corner_cycles == 0 && status.edge_cycles == 0 && status.center_cycles == 0) {
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
                if (status.center_cycles <= 1 && status.parity) {
                    if (status.corner_cycles || status.edge_cycles) {
                        cout << " with parity";
                    } else {
                        cout << "parity";
                    }
                }
            }
            cout << '.' << endl;
        }

        void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            const CycleStatus& status,
            const Finder& finder, const Finder::Result& result
        ) override {
            if (result.status == FinderStatus::success) {
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
            } else {
                if (result.status == FinderStatus::parity_algorithms_needed) {
                    cout << "Parity algorithms needed." << endl;
                }
                if (result.status == FinderStatus::corner_cycle_algorithms_needed) {
                    cout << "Corner cycle algorithms needed." << endl;
                }
                if (result.status == FinderStatus::edge_cycle_algorithms_needed) {
                    cout << "Edge cycle algorithms needed." << endl;
                }
                if (result.status == FinderStatus::center_algorithms_needed) {
                    cout << "Center algorithms needed." << endl;
                }
            }
            cout << "Time usage: " << fixed << setprecision(3);
            if (result.duration < 1000) {
                cout << result.duration << " nanoseconds." << endl;
            } else if (result.duration < 1'000'000) {
                cout << result.duration / 1e3 << " microseconds." << endl;
            } else if (result.duration < 1'000'000'000) {
                cout << result.duration / 1e6 << " milliseconds." << endl;
            } else if (result.duration < 60 * static_cast<int64_t>(1'000'000'000)) {
                cout << result.duration / 1e9 << " seconds." << endl;
            } else if (result.duration < 60 * 60 * static_cast<int64_t>(1'000'000'000)) {
                int64_t duration = (result.duration + 500'000) / 1'000'000;
                cout << duration / (60 * 1000)
                    << right << setfill('0')
                    << ':' << setw(2) << duration / 1000 % 60
                     << '.'<< setw(3) << duration % 1000 << endl;
            } else {
                int64_t duration = (result.duration + 500'000) / 1'000'000;
                cout << duration / (60 * 60 * 1000)
                    << right << setfill('0')
                    << ':' << setw(2) << duration / (60 * 1000) % 60
                    << ':' << setw(2) << duration / 1000 % 60
                    << ':' << setw(3) << duration % 1000 << '.' << endl;
            }
        }
    };

    struct JSONPrinter: Printer {
        void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            const CycleStatus& status,
            const Finder& finder, const Finder::Result& result
        ) override {
            UniValue map(UniValue::VOBJ);
            map.pushKV("scramble", scramble.str());
            map.pushKV("skeleton", skeleton.str());
            map.pushKV("parity", status.parity);
            map.pushKV("corner_cycles", status.corner_cycles);
            map.pushKV("edge_cycles", status.edge_cycles);
            map.pushKV("center_cycles", status.center_cycles);
            const auto& solutions = finder.get_solutions();
            if (!status.parity && status.corner_cycles == 0 && status.edge_cycles == 0 && status.center_cycles == 0) {
                map.pushKV("fewest_moves", static_cast<int>(skeleton.length()));
            } else if (solutions.empty()) {
                map.pushKV("fewest_moves", UniValue());
            } else {
                map.pushKV("fewest_moves", static_cast<int>(finder.get_fewest_moves()));
            }

            UniValue solution_list(UniValue::VARR);
            for (const auto& solution: solutions) {
                UniValue solution_map(UniValue::VOBJ);
                solution_map.pushKV("final_solution", solution.insertions.back().skeleton.str());
                solution_map.pushKV("cancellation", static_cast<int>(solution.cancellation));
                UniValue insertion_list(UniValue::VARR);
                for (size_t depth = 0; depth < solution.insertions.size() - 1; ++depth) {
                    const auto& insertion = solution.insertions[depth];
                    UniValue insertion_map(UniValue::VOBJ);
                    insertion_map.pushKV("skeleton", insertion.skeleton.str());
                    insertion_map.pushKV("insert_place", static_cast<int>(insertion.insert_place));
                    insertion_map.pushKV("insertion", insertion.insertion->str());
                    insertion_list.push_back(insertion_map);
                }
                solution_map.pushKV("insertions", insertion_list);
                solution_list.push_back(solution_map);
            }
            map.pushKV("solutions", solution_list);

            map.pushKV("duration", result.duration);
            cout << map.write() << flush;
        }
    };
};


void CLI::find_insertions(const po::variables_map& vm) {
    const vector<string> filenames = vm.count("file") ? vm["file"].as<vector<string>>() : vector<string>();
    const fs::path algorithms_directory = vm["algs-dir"].as<string>();
    vector<string> algfilenames = vm.count("algfile") ? vm["algfile"].as<vector<string>>() : vector<string>();
    if (vm.count("all-algs")) {
        try {
            for (auto& file: fs::directory_iterator(algorithms_directory)) {
                if (fs::is_regular_file(file) && file.path().extension() == ".algs") {
                    algfilenames.emplace_back(file.path().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            cerr << e.what() << endl;
        }
    }
    if (vm.count("all-extra-algs")) {
        try {
            for (auto &file : fs::directory_iterator(algorithms_directory / "extras")) {
                if (fs::is_regular_file(file) && file.path().extension() == ".algs") {
                    algfilenames.emplace_back(file.path().string());
                }
            }
        } catch (const fs::filesystem_error &e) {
            cerr << e.what() << endl;
        }
    }

    unordered_map<Cube, Case> map;
    for (const string& name: algfilenames) {
        ifstream fin(name, ios::in | ios::binary);
        if (fin.fail()) {
            fin = ifstream(
                (algorithms_directory / (name + ".algs")).string(),
                ios::in | ios::binary
            );
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
            auto [node, inserted] = map.try_emplace(_case.state(), move(_case));
            if (!inserted) {
                node->second.merge_algorithms(move(_case));
            }
        }
    }

    vector<Case> cases;
    for (auto& node: map) {
        cases.emplace_back(move(node.second));
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

    unique_ptr<Printer> printer;
    if (vm.count("json")) {
        printer = make_unique<JSONPrinter>();
    } else {
        printer = make_unique<StandardPrinter>();
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

    Cube original_cube;
    original_cube.twist(scramble);
    original_cube.twist(skeleton);
    Cube cube = original_cube.best_placement();
    bool parity = cube.has_parity();
    int corner_cycles = cube.corner_cycles();
    int edge_cycles = cube.edge_cycles();
    int center_cycles = Cube::center_cycles[cube.placement()];
    printer->print_case_information(scramble, skeleton, {parity, corner_cycles, edge_cycles, center_cycles});

    unique_ptr<Finder> finder;
    if (vm.count("optimal")) {
        finder = make_unique<BruteForceFinder>(scramble, skeleton, cases);
    } else {
        finder = make_unique<GreedyFinder>(
            scramble, skeleton, cases,
            GreedyFinder::Options {
                static_cast<bool>(vm.count("enable-replacement")),
                vm["greedy-threshold"].as<size_t>(),
                vm["replacement-threshold"].as<size_t>()
            }
        );
    }

    if (vm.count("verbose")) {
        finder->set_verbose();
    }
    size_t search_target = numeric_limits<size_t>::max();
    if (vm.count("target")) {
        search_target = vm["target"].as<size_t>();
    }
    size_t max_threads = vm["jobs"].as<size_t>();
    double parity_multiplier = vm["parity"].as<double>();
    if (parity_multiplier != 1) {
        parity_multiplier = 1.5;
    }
    finder->search({search_target, parity_multiplier, max_threads});
    printer->print_result(
        scramble, skeleton,
        {parity, corner_cycles, edge_cycles, center_cycles},
        *finder, finder->get_result()
    );
}
