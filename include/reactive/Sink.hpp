// include/reactive/Sink.hpp
#pragma once

#include "Models.hpp"

#include <rxcpp/rx.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>      // <-- THÊM
#include <stdexcept>
#include <string>

namespace reactive {

// ============================================================================
// GLOBAL MUTEX cho console output
// ============================================================================
inline std::mutex& consoleMutex() {
    static std::mutex mtx;
    return mtx;
}

// ============================================================================
// ConsoleSink
// ============================================================================
template <typename T>
class ConsoleSink {
public:
    using Formatter = std::function<std::string(const T&)>;

    explicit ConsoleSink(Formatter formatter = nullptr,
                         std::ostream& out = std::cout,
                         bool useColor = true)
        : formatter_(formatter ? std::move(formatter)
                               : [](const T& v) { return defaultFormat(v); })
        , out_(&out)
        , useColor_(useColor)
    {}

    auto makeObserver() {
        return rxcpp::make_observer<T>(
            [this](const T& value) {
                std::lock_guard<std::mutex> lock(consoleMutex());   // <-- KHÓA
                const std::string s = formatter_(value);
                *out_ << (useColor_ ? colorize(s) : s) << '\n';
                out_->flush();
            },
            [this](std::exception_ptr e) {
                std::lock_guard<std::mutex> lock(consoleMutex());
                try {
                    if (e) std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    *out_ << "[ERROR] " << ex.what() << '\n';
                }
            },
            [this]() {
                std::lock_guard<std::mutex> lock(consoleMutex());
                *out_ << "[DONE] Console sink hoan tat\n";
                out_->flush();
            }
        );
    }

private:
    Formatter     formatter_;
    std::ostream* out_;
    bool          useColor_;

    static std::string defaultFormat(const T& v) {
        if constexpr (requires { v.toString(); }) {
            return v.toString();
        } else if constexpr (requires { v.toJson(); }) {
            return v.toJson();
        } else {
            return std::string{"<unformatted>"};
        }
    }

    static std::string colorize(const std::string& s) {
        if (s.find("[CRITICAL]") != std::string::npos)
            return "\033[1;31m" + s + "\033[0m";
        if (s.find("[WARNING]") != std::string::npos)
            return "\033[1;33m" + s + "\033[0m";
        if (s.find("[INFO]") != std::string::npos)
            return "\033[1;32m" + s + "\033[0m";
        return s;
    }
};

// ============================================================================
// FileSink
// ============================================================================
template <typename T>
class FileSink {
public:
    enum class Format { CSV, JSON, RAW };

    explicit FileSink(const std::string& path, Format format = Format::CSV)
        : path_(path)
        , format_(format)
    {
        file_.open(path, std::ios::out | std::ios::app | std::ios::binary);
        if (!file_.is_open()) {
            throw std::runtime_error("FileSink: khong the mo file " + path_);
        }
    }

    ~FileSink() {
        try {
            if (file_.is_open()) file_.flush();
        } catch (...) {}
    }

    FileSink(const FileSink&) = delete;
    FileSink& operator=(const FileSink&) = delete;

    auto makeObserver() {
        return rxcpp::make_observer<T>(
            [this](const T& value) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (file_.is_open()) {
                    file_ << formatValue(value) << '\n';
                }
            },
            [this](std::exception_ptr e) {
                std::lock_guard<std::mutex> lock(mutex_);
                try {
                    if (e) std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    if (file_.is_open()) {
                        file_ << "ERROR," << ex.what() << '\n';
                    }
                }
            },
            [this]() {
                std::lock_guard<std::mutex> lock(mutex_);
                if (file_.is_open()) file_.flush();
            }
        );
    }

private:
    std::string  path_;
    Format       format_;
    std::ofstream file_;
    std::mutex   mutex_;

    std::string formatValue(const T& v) {
        switch (format_) {
            case Format::CSV:
                if constexpr (requires { v.toCsv(); }) return v.toCsv();
                break;
            case Format::JSON:
                if constexpr (requires { v.toJson(); }) return v.toJson();
                break;
            case Format::RAW:
            default:
                break;
        }
        return std::string{"<no format>"};
    }
};

} // namespace reactive