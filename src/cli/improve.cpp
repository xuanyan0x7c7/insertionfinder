#include <cstddef>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <boost/program_options.hpp>
#include <univalue.h>
#include <insertionfinder/fallbacks/filesystem.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/termcolor.hpp>
#include <insertionfinder/improver/improver.hpp>
#include <insertionfinder/improver/slice.hpp>
#include "../utils/encoding.hpp"
#include "commands.hpp"
#include "utils.hpp"
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
    struct Printer {
        virtual void print_case_information(const Algorithm& skeleton) {}
        virtual void print_result(
            const Algorithm& skeleton,
            const Improver& improver, Improver::Result result,
            bool expand
        ) = 0;
        virtual ~Printer() = default;
    };

    struct StandardPrinter: Printer {
        void print_case_information(const Algorithm& skeleton) override {
            std::cout << termcolor::bold << "Skeleton: " << termcolor::reset << skeleton;
            if (skeleton.length()) {
                std::cout << " (" << termcolor::yellow << skeleton.length() << termcolor::reset << "f)";
            }
            std::cout << std::endl;
        }

        void print_result(
            const Algorithm& skeleton,
            const Improver& improver, Improver::Result result,
            bool expand
        ) override {
            if (improver.get_fewest_moves() == skeleton.length()) {
                std::cout << "No improvements found." << std::endl;
            } else {
                const auto& solutions = improver.get_solutions();
                for (size_t index = 0; index < solutions.size(); ++index) {
                    const Solution& solution = solutions[index];
                    std::cout << std::endl
                        << termcolor::bold << "Improvement #" << index + 1 << termcolor::reset << std::endl;
                    if (expand) {
                        std::cout << solution;
                    } else {
                        solution.print(std::cout, solution.merge_insertions(skeleton));
                    }
                    std::cout << std::endl;
                }
            }
            Details::print_duration(std::cout, result.duration);
        }
    };

    struct JSONPrinter: Printer {
        void print_result(
            const Algorithm& skeleton,
            const Improver& improver, Improver::Result result,
            bool expand
        ) override {
            UniValue map(UniValue::VOBJ);
            map.pushKV("skeleton", skeleton.str());
            const auto& solutions = improver.get_solutions();
            map.pushKV("fewest_moves", static_cast<int>(improver.get_fewest_moves()));
            UniValue solution_list(UniValue::VARR);
            for (const Solution& solution: solutions) {
                solution_list.push_back(Details::create_json_solution(skeleton, solution));
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
            auto [node, inserted] = map.try_emplace(_case.get_state(), std::move(_case));
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
    std::sort(cases.begin(), cases.end(), [](const Case& x, const Case& y) {return Case::compare(x, y) < 0;});

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
    size_t max_threads = vm["jobs"].as<size_t>();
    improver->search({max_threads});
    printer->print_result(skeleton, *improver, improver->get_result(), vm.count("expand-insertions"));
}
