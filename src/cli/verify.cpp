#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include "commands.hpp"
#include "univalue/univalue.h"
namespace po = boost::program_options;
using InsertionFinder::Algorithm;
using InsertionFinder::AlgorithmError;
using InsertionFinder::Cube;
namespace CLI = InsertionFinder::CLI;


namespace {
    struct CycleStatus {
        Algorithm scramble;
        Algorithm skeleton;
        bool parity;
        int corner_cycles;
        int edge_cycles;
        int center_cycles;
    };

    CycleStatus verify(const std::string& scramble_string, const std::string& skeleton_string) try {
        Algorithm scramble(scramble_string);
        Algorithm skeleton(skeleton_string);
        scramble.simplify();
        skeleton.simplify();
        Cube cube = Cube() * scramble * skeleton;
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
    struct Printer {
        virtual void print_result(const CycleStatus& status) = 0;
        virtual ~Printer() = default;
    };

    struct StandardPrinter: Printer {
        void print_result(const CycleStatus& status) override {
            std::cout << "Scramble: " << status.scramble << std::endl;
            std::cout << "Skeleton: " << status.skeleton << std::endl;
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
    };

    struct JSONPrinter: Printer {
        void print_result(const CycleStatus& status) override {
            UniValue map(UniValue::VOBJ);
            map.pushKV("scramble", status.scramble.str());
            map.pushKV("skeleton", status.skeleton.str());
            map.pushKV("parity", status.parity);
            map.pushKV("corner_cycles", status.corner_cycles);
            map.pushKV("edge_cycles", status.edge_cycles);
            map.pushKV("center_cycles", status.center_cycles);
            std::cout << map.write() << std::flush;
        }
    };
};


void CLI::verify_cube(const po::variables_map& vm) {
    std::unique_ptr<Printer> printer;
    if (vm.count("json")) {
        printer = std::make_unique<JSONPrinter>();
    } else {
        printer = std::make_unique<StandardPrinter>();
    }
    std::string scramble_string;
    std::string skeleton_string;
    std::getline(std::cin, scramble_string);
    std::getline(std::cin, skeleton_string);
    auto status = verify(scramble_string, skeleton_string);
    printer->print_result(status);
}
