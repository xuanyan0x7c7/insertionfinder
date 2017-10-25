#pragma once
#include <exception>
#include <string>

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

    template<class T> void verify_cube();
};
