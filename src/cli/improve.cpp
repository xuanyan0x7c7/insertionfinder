#include <iomanip>
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
#include <insertionfinder/improver/improver.hpp>
#include <insertionfinder/improver/slice.hpp>
#include "../utils/encoding.hpp"
#include "commands.hpp"
using std::int64_t;
using std::size_t;
namespace fs = std::filesystem;
namespace po = boost::program_options;
using InsertionFinder::Algorithm;
using InsertionFinder::Case;
using InsertionFinder::Cube;
using InsertionFinder::Improver;
using InsertionFinder::Insertion;
using InsertionFinder::SliceImprover;
using InsertionFinder::Solution;
namespace CLI = InsertionFinder::CLI;
namespace Details = InsertionFinder::Details;


namespace {
    struct CycleStatus {
        bool parity;
        int corner_cycles;
        int edge_cycles;
        int center_cycles;
    };

    struct Printer {
        virtual void print_case_information(const Algorithm& skeleton) {}
        virtual void print_result(
            const Algorithm& skeleton,
            const Improver& improver, const Improver::Result& result
        ) = 0;
        virtual ~Printer() = default;
    };

    struct StandardPrinter: Printer {
        void print_case_information(const Algorithm& skeleton) override {
            std::cout << "Skeleton: " << skeleton << std::endl;
        }

        void print_result(
            const Algorithm& skeleton,
            const Improver& improver, const Improver::Result& result
        ) override {
            const auto& solutions = improver.get_solutions();
            if (improver.get_fewest_moves() == skeleton.length()) {
                std::cout << "No improvements found." << std::endl;
            } else {
                for (size_t index = 0; index < solutions.size(); ++index) {
                    const Solution& solution = solutions[index];
                    std::cout << std::endl << "Improvement #" << index + 1 << std::endl;
                    for (size_t i = 0; i < solution.insertions.size() - 1; ++i) {
                        const Insertion& insertion = solution.insertions[i];
                        const Algorithm& skeleton = insertion.skeleton;
                        size_t insert_place = insertion.insert_place;
                        if (insert_place > 0) {
                            skeleton.print(std::cout, 0, insert_place);
                            std::cout << ' ';
                        }
                        std::cout << "[@" << i + 1 << ']';
                        if (insert_place < skeleton.length()) {
                            std::cout << ' ';
                            skeleton.print(std::cout, insert_place, skeleton.length());
                        }
                        std::cout << std::endl;
                        std::cout << "Insert at @" << i + 1 << ": " << *insertion.insertion << std::endl;
                    }
                    std::cout
                        << "Total moves: " << improver.get_fewest_moves() << ", "
                        << solution.cancellation << " move" << (solution.cancellation == 1 ? "" : "s")
                        << " cancelled." << std::endl
                        << "Full solution: " << solution.insertions.back().skeleton << std::endl;
                }
            }
            std::cout << "Time usage: " << std::fixed << std::setprecision(3);
            if (result.duration < 1000) {
                std::cout << result.duration << " nanoseconds." << std::endl;
            } else if (result.duration < 1'000'000) {
                std::cout << result.duration / 1e3 << " microseconds." << std::endl;
            } else if (result.duration < 1'000'000'000) {
                std::cout << result.duration / 1e6 << " milliseconds." << std::endl;
            } else if (result.duration < 60 * static_cast<int64_t>(1'000'000'000)) {
                std::cout << result.duration / 1e9 << " seconds." << std::endl;
            } else if (result.duration < 60 * 60 * static_cast<int64_t>(1'000'000'000)) {
                int64_t duration = (result.duration + 500'000) / 1'000'000;
                std::cout << duration / (60 * 1000)
                    << std::right << std::setfill('0')
                    << ':' << std::setw(2) << duration / 1000 % 60
                     << '.'<< std::setw(3) << duration % 1000 << std::endl;
            } else {
                int64_t duration = (result.duration + 500'000) / 1'000'000;
                std::cout << duration / (60 * 60 * 1000)
                    << std::right << std::setfill('0')
                    << ':' << std::setw(2) << duration / (60 * 1000) % 60
                    << ':' << std::setw(2) << duration / 1000 % 60
                    << ':' << std::setw(3) << duration % 1000 << '.' << std::endl;
            }
        }
    };

    struct JSONPrinter: Printer {
        void print_result(
            const Algorithm& skeleton,
            const Improver& improver, const Improver::Result& result
        ) override {
            UniValue map(UniValue::VOBJ);
            map.pushKV("skeleton", skeleton.str());
            const auto& solutions = improver.get_solutions();
            map.pushKV("fewest_moves", static_cast<int>(improver.get_fewest_moves()));

            UniValue solution_list(UniValue::VARR);
            for (const Solution& solution: solutions) {
                UniValue solution_map(UniValue::VOBJ);
                solution_map.pushKV("final_solution", solution.insertions.back().skeleton.str());
                solution_map.pushKV("cancellation", static_cast<int>(solution.cancellation));
                UniValue insertion_list(UniValue::VARR);
                for (size_t depth = 0; depth < solution.insertions.size() - 1; ++depth) {
                    const Insertion& insertion = solution.insertions[depth];
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


void CLI::find_improvements(const po::variables_map& vm) {
    const std::vector<std::string> filenames =
        vm.count("file") ? vm["file"].as<std::vector<std::string>>() : std::vector<std::string>();
    const fs::path algorithms_directory = vm["algs-dir"].as<std::string>();
    std::vector<std::string> algfilenames =
        vm.count("algfile") ? vm["algfile"].as<std::vector<std::string>>() : std::vector<std::string>();
    if (vm.count("all-algs")) {
        try {
            for (const auto& file : fs::directory_iterator(algorithms_directory / "slices")) {
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
    Algorithm skeleton;
    try {
        skeleton = Algorithm(line);
    } catch (const AlgorithmError& e) {
        throw CLI::CommandExecutionError("Invalid skeleton " + line);
    }
    in.reset();
    skeleton.simplify();

    std::unique_ptr<Printer> printer;
    if (vm.count("json")) {
        printer = std::make_unique<JSONPrinter>();
    } else {
        printer = std::make_unique<StandardPrinter>();
    }
    printer->print_case_information(skeleton);

    std::unique_ptr<Improver> improver = std::make_unique<SliceImprover>(
        skeleton, cases,
        SliceImprover::Options {vm["replacement-threshold"].as<size_t>()}
    );
    // if (vm.count("verbose")) {
    //     improver->set_verbose();
    // }
    size_t max_threads = vm["jobs"].as<size_t>();
    improver->search({max_threads});
    printer->print_result(skeleton, *improver, improver->get_result());
}
