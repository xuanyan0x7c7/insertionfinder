#include <cstddef>
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>
#include <boost/program_options.hpp>
#include <univalue.h>
#include <insertionfinder/fallbacks/filesystem.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/finder/brute-force.hpp>
#include <insertionfinder/finder/greedy.hpp>
#include "../utils/encoding.hpp"
#include "commands.hpp"
#include "utils.hpp"
using std::size_t;
namespace fs = std::filesystem;
namespace po = boost::program_options;
using InsertionFinder::Algorithm;
using InsertionFinder::BruteForceFinder;
using InsertionFinder::Case;
using InsertionFinder::Cube;
using InsertionFinder::GreedyFinder;
using InsertionFinder::Finder;
using InsertionFinder::Insertion;
using InsertionFinder::Solution;
namespace CLI = InsertionFinder::CLI;
namespace Details = InsertionFinder::Details;
namespace FinderStatus = InsertionFinder::FinderStatus;


namespace {
    struct CycleStatus {
        bool parity;
        int corner_cycles;
        int edge_cycles;
        int center_cycles;
    };

    struct Printer {
        virtual void print_case_information(const Algorithm& scramble, const Algorithm& skeleton, CycleStatus status) {}
        virtual void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            CycleStatus status,
            const Finder& finder, Finder::Result result
        ) = 0;
        virtual ~Printer() = default;
    };

    struct StandardPrinter: Printer {
        void print_case_information(const Algorithm& scramble, const Algorithm& skeleton, CycleStatus status) override {
            std::cout << "Scramble: " << scramble << std::endl;
            std::cout << "Skeleton: " << skeleton << std::endl;
            std::cout << "The cube ";
            if (!status.parity && status.corner_cycles == 0 && status.edge_cycles == 0 && status.center_cycles == 0) {
                std::cout << "is already solved";
            } else {
                std::cout << "has ";
                if (status.center_cycles) {
                    if (status.center_cycles > 1) {
                        std::cout << "parity center rotation";
                    } else {
                        std::cout << "center rotation";
                    }
                    if (status.corner_cycles || status.edge_cycles) {
                        std::cout << " with ";
                    }
                }
                if (status.corner_cycles == 1) {
                    std::cout << "1 corner-3-cycle";
                } else if (status.corner_cycles > 1) {
                    std::cout << status.corner_cycles << " corner-3-cycles";
                }
                if (status.corner_cycles && status.edge_cycles) {
                    std::cout << " and ";
                }
                if (status.edge_cycles == 1) {
                    std::cout << "1 edge-3-cycle";
                } else if (status.edge_cycles > 1) {
                    std::cout << status.edge_cycles << " edge-3-cycles";
                }
                if (status.center_cycles <= 1 && status.parity) {
                    if (status.corner_cycles || status.edge_cycles) {
                        std::cout << " with parity";
                    } else {
                        std::cout << "parity";
                    }
                }
            }
            std::cout << '.' << std::endl;
        }

        void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            CycleStatus status,
            const Finder& finder, Finder::Result result
        ) override {
            if (result.status == FinderStatus::success) {
                const auto& solutions = finder.get_solutions();
                if (
                    solutions.empty() &&
                    (status.parity || status.corner_cycles || status.edge_cycles || status.center_cycles)
                ) {
                    std::cout << "No solution found." << std::endl;
                }
                for (size_t index = 0; index < solutions.size(); ++index) {
                    const Solution& solution = solutions[index];
                    std::cout << std::endl << "Solution #" << index + 1 << std::endl << solution << std::endl;
                }
            } else {
                if (result.status == FinderStatus::parity_algorithms_needed) {
                    std::cout << "Parity algorithms needed." << std::endl;
                }
                if (result.status == FinderStatus::corner_cycle_algorithms_needed) {
                    std::cout << "Corner cycle algorithms needed." << std::endl;
                }
                if (result.status == FinderStatus::edge_cycle_algorithms_needed) {
                    std::cout << "Edge cycle algorithms needed." << std::endl;
                }
                if (result.status == FinderStatus::center_algorithms_needed) {
                    std::cout << "Center algorithms needed." << std::endl;
                }
            }
            Details::print_duration(std::cout, result.duration);
        }
    };

    struct JSONPrinter: Printer {
        void print_result(
            const Algorithm& scramble, const Algorithm& skeleton,
            CycleStatus status,
            const Finder& finder, Finder::Result result
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
            for (const Solution& solution: solutions) {
                UniValue solution_map(UniValue::VOBJ);
                solution_map.pushKV("final_solution", solution.final_solution.str());
                solution_map.pushKV("cancellation", static_cast<int>(solution.cancellation));
                UniValue insertion_list(UniValue::VARR);
                for (const Insertion& insertion: solution.insertions) {
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
            std::cout << map.write() << std::flush;
        }
    };
};


void CLI::find_insertions(const po::variables_map& vm) {
    const std::vector<std::string> filenames =
        vm.count("file") ? vm["file"].as<std::vector<std::string>>() : std::vector<std::string>();
    const fs::path algorithms_directory = vm["algs-dir"].as<std::string>();
    std::vector<std::string> algfilenames =
        vm.count("algfile") ? vm["algfile"].as<std::vector<std::string>>() : std::vector<std::string>();
    if (vm.count("all-algs")) {
        try {
            for (const auto& file: fs::directory_iterator(algorithms_directory)) {
                if (fs::is_regular_file(file) && file.path().extension() == ".algs") {
                    algfilenames.emplace_back(file.path().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    if (vm.count("all-extra-algs")) {
        try {
            for (const auto& file : fs::directory_iterator(algorithms_directory / "extras")) {
                if (fs::is_regular_file(file) && file.path().extension() == ".algs") {
                    algfilenames.emplace_back(file.path().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    std::unordered_map<Cube, Case> map;
    for (const std::string& name: algfilenames) {
        std::ifstream fin(name, std::ios::in | std::ios::binary);
        if (fin.fail()) {
            fin = std::ifstream((algorithms_directory / (name + ".algs")).string(), std::ios::in | std::ios::binary);
            if (fin.fail()) {
                std::cerr << "Failed to open algorithm file " << name << std::endl;
                continue;
            }
        }
        size_t size;
        if (auto x = Details::read_varuint(fin)) {
            size = *x;
        } else {
            std::cerr << "Invalid algorithm file " << name << std::endl;
            continue;
        }
        for (size_t i = 0; i < size; ++i) {
            Case _case;
            try {
                _case.read_from(fin);
            } catch (...) {
                std::cerr << "Invalid algorithm file " << name << std::endl;
                break;
            }
            auto [node, inserted] = map.try_emplace(_case.state(), std::move(_case));
            if (!inserted) {
                node->second.merge_algorithms(std::move(_case));
            }
        }
    }

    std::vector<Case> cases;
    cases.reserve(map.size());
    for (auto& node: map) {
        cases.emplace_back(std::move(node.second));
    }
    ranges::sort(cases, [](const Case& x, const Case& y) {return Case::compare(x, y) < 0;});

    std::shared_ptr<std::istream> in;
    if (filenames.empty()) {
        in.reset(&std::cin, [](std::istream*) {});
    } else {
        const std::string& name = filenames.front();
        in = std::make_shared<std::ifstream>(name);
        if (in->fail()) {
            throw CLI::CommandExecutionError("Failed to open input file " + name);
        }
    }

    std::string line;
    std::getline(*in, line);
    Algorithm scramble;
    try {
        scramble = Algorithm(line);
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError("Invalid scramble " + line);
    }
    std::getline(*in, line);
    Algorithm skeleton;
    try {
        skeleton = Algorithm(line);
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError("Invalid skeleton " + line);
    }
    in.reset();

    scramble.simplify();
    skeleton.simplify();
    Cube original_cube = Cube() * scramble * skeleton;
    Cube cube = original_cube.best_placement();
    bool parity = cube.has_parity();
    int corner_cycles = cube.corner_cycles();
    int edge_cycles = cube.edge_cycles();
    int center_cycles = Cube::center_cycles[cube.placement()];

    std::unique_ptr<Printer> printer;
    if (vm.count("json")) {
        printer = std::make_unique<JSONPrinter>();
    } else {
        printer = std::make_unique<StandardPrinter>();
    }
    printer->print_case_information(scramble, skeleton, {parity, corner_cycles, edge_cycles, center_cycles});

    std::unique_ptr<Finder> finder;
    if (vm.count("optimal")) {
        finder = std::make_unique<BruteForceFinder>(scramble, skeleton, cases);
    } else {
        finder = std::make_unique<GreedyFinder>(
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
    size_t search_target = std::numeric_limits<size_t>::max();
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
