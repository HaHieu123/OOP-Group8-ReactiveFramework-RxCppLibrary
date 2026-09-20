// include/reactive/Sink.hpp
#pragma once

#include "Models.hpp"

#include <rxcpp/rx.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

namespace reactive {

// ============================================================================
// Sink: wrapper cho rxcpp::subscriber
// ============================================================================

// ----------------------------------------------------------------------------
// ConsoleSink: in ra màn hình
// ----------------------------------------------------------------------------
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

    // Trả về observer cho RxCpp
    auto makeObserver() {
        return rxcpp::make_observer<T>(
            // onNext
            [this](const T& value) {
                std::lock_guard<std::mutex> lock(mutex_);
                const std::string s = formatter_(value);
                *out_ << (useColor_ ? colorize(s) : s) << '\n';
            },
            // onError
            [this](std::exception_ptr e) {
                std::lock_guard<std::mutex> lock(mutex_);
                try {
                    if (e) std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    *out_ << "[ERROR] " << ex.what() << '\n';
                }
            },
            // onComplete
            [this]() {
                std::lock_guard<std::mutex> lock(mutex_);
                *out_ << "[DONE] Console sink hoan tat\n";
                out_->flush();
            }
        );
    }

private:
    Formatter     formatter_;
    std::ostream* out_;
    bool          useColor_;
    std::mutex    mutex_;

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

// ----------------------------------------------------------------------------
// FileSink: ghi ra file CSV/JSON
// ----------------------------------------------------------------------------
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
        if (file_.is_open()) file_.flush();
    }

    auto makeObserver() {
        return rxcpp::make_observer<T>(
            [this](const T& value) {
                std::lock_guard<std::mutex> lock(mutex_);
                file_ << formatValue(value) << '\n';
            },
            [this](std::exception_ptr e) {
                std::lock_guard<std::mutex> lock(mutex_);
                try {
                    if (e) std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    file_ << "ERROR," << ex.what() << '\n';
                }
            },
            [this]() {
                std::lock_guard<std::mutex> lock(mutex_);
                file_.flush();
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
                break;
        }
        return std::string{"<no format>"};
    }
};

} // namespace reactive