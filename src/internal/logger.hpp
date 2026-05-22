#pragma once

#include <fstream>
#include <iostream>
#include <string>

class Logger {
public:
    explicit Logger(const std::string& path, bool also_stdout = false)
        : stdout_enabled_(also_stdout || path == "-") {
        if (path != "-") {
            output_.open(path, std::ios::out | std::ios::trunc);
        }
    }

    [[nodiscard]] bool is_open() const {
        return stdout_enabled_ || output_.is_open();
    }

    void log(const std::string& line) {
        if (output_.is_open()) {
            output_ << line << '\n';
        }
        if (stdout_enabled_) {
            std::cout << line << '\n';
        }
    }

    void flush() {
        if (output_.is_open()) {
            output_.flush();
        }
        if (stdout_enabled_) {
            std::cout.flush();
        }
    }

private:
    std::ofstream output_;
    bool stdout_enabled_ = false;
};
