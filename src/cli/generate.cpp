#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/program_options.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include "commands.hpp"
using std::size_t;
namespace po = boost::program_options;
using InsertionFinder::Algorithm;
using InsertionFinder::AlgorithmError;
using InsertionFinder::Case;
using InsertionFinder::Cube;
namespace CLI = InsertionFinder::CLI;


void CLI::generate_algorithms(const po::variables_map& vm) {
    const std::vector<std::string> filenames =
        vm.count("file") ? vm["file"].as<std::vector<std::string>>() : std::vector<std::string>();
    const std::vector<std::string> algfilenames =
        vm.count("algfile") ? vm["algfile"].as<std::vector<std::string>>() : std::vector<std::string>();

    std::unordered_map<Cube, Case> map;

    for (const std::string& name: filenames) {
        std::ifstream fin(name);
        if (fin.fail()) {
            std::cerr << "Failed to open file " << name << std::endl;
            continue;
        }
        while (!fin.eof()) {
            std::string line;
            std::getline(fin, line);
            Algorithm algorithm;
            try {
                algorithm = Algorithm(line);
            } catch (const AlgorithmError& e) {
                std::cerr << "Invalid algorithm: " << e.what() << std::endl;
                continue;
            }
            algorithm.simplify();
            algorithm.normalize();
            algorithm.detect_rotation();
            Cube cube = Cube() * algorithm;
            if (cube.mask() == 0) {
                continue;
            }

            if (auto node = map.find(cube); node != map.end() && node->second.contains_algorithm(algorithm)) {
                continue;
            }
            auto isomorphism_list = algorithm.generate_isomorphisms();
            for (Algorithm& algorithm: isomorphism_list) {
                Cube cube = Cube() * algorithm;
                if (auto node = map.find(cube); node != map.end()) {
                    node->second.add_algorithm(std::move(algorithm));
                } else {
                    Case _case(cube);
                    _case.add_algorithm(std::move(algorithm));
                    map.emplace(cube, std::move(_case));
                }
            }
        }
    }

    size_t size = map.size();
    std::vector<Case> cases;
    cases.reserve(size);
    for (auto& node: map) {
        cases.emplace_back(std::move(node.second));
    }
    std::sort(cases.begin(), cases.end(), [](const Case& x, const Case& y) {return Case::compare(x, y) < 0;});

    std::shared_ptr<std::ostream> out;
    if (algfilenames.empty()) {
        out.reset(&std::cout, [](std::ostream*) {});
    } else {
        const std::string& name = algfilenames.front();
        out = std::make_shared<std::ofstream>(name, std::ios::out | std::ios::binary);
        if (out->fail()) {
            throw CLI::CommandExecutionError("Failed to open output file" + name);
        }
    }
    out->write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    for (auto& _case: cases) {
        _case.sort_algorithms();
        _case.save_to(*out);
    }
}
