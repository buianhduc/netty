#pragma once

#include <fstream>
#include <string>

class Logger {
public:
    explicit Logger(const std::string& path) : output_(path, std::ios::out | std::ios::trunc) {}

    [[nodiscard]] bool is_open() const {
        return output_.is_open();
    }

    void log(const std::string& line) {
        output_ << line << '\n';
    }

private:
    std::ofstream output_;
};
