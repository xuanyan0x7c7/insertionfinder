#pragma once
#include <exception>
#include <string>
#include <boost/program_options.hpp>

namespace InsertionFinder::CLI {
    class CommandExecutionError: std::exception {
    private:
        const std::string message;
    public:
        CommandExecutionError(std::string message): message(message) {}
        virtual const char* what() const noexcept override {
            return message.c_str();
        }
    };

    void verify_cube(const boost::program_options::variables_map& vm);
    void generate_algorithms(const boost::program_options::variables_map& vm);
    void find_insertions(const boost::program_options::variables_map& vm);
    void find_improvements(const boost::program_options::variables_map& vm);
};
