#pragma once
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

static inline std::string timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream sstream;
    sstream << std::put_time(std::localtime(&now_t), "%Y-%m-%d-%H.%M");
    
    return sstream.str();
}

template<typename T>
T parse(const std::string& s) {
    T v;
    std::stringstream sstream(s);
    sstream >> v;
    return v;
}


