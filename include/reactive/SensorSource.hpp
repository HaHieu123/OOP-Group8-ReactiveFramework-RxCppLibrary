// include/reactive/SensorSource.hpp
#pragma once

#include "Models.hpp"

#include <rxcpp/rx.hpp>

#include <chrono>
#include <memory>
#include <random>

namespace reactive {

// ============================================================================
// GLOBAL SCHEDULER — dùng chung cho tất cả SensorSource
// ============================================================================
// Tạo MỘT LẦN, tái sử dụng → đảm bảo thread không bị hủy sớm
// ============================================================================
inline rxcpp::observe_on_one_worker& globalScheduler() {
    static auto scheduler = rxcpp::observe_on_new_thread();
    return scheduler;
}

// ============================================================================
// SensorSource
// ============================================================================
class SensorSource {
public:
    SensorSource(Sensor sensor,
                 double baseTemperature,
                 double noiseLevel,
                 std::chrono::milliseconds interval)
        : sensor_(std::move(sensor))
        , interval_(interval)
        , baseTemp_(baseTemperature)
        , noise_(noiseLevel)
    {}

    rxcpp::observable<Reading> asObservable() {
        // Copy tất cả vào shared_ptr
        auto sensorCopy = std::make_shared<Sensor>(sensor_);
        auto baseTemp   = baseTemp_;
        auto noise      = noise_;
        auto interval   = interval_;

        // Random generator
        auto rng = std::make_shared<std::mt19937>(std::random_device{}());

        // DÙNG GLOBAL SCHEDULER — không tạo mới
        auto scheduler = globalScheduler();

        return rxcpp::observable<>::interval(
            std::chrono::steady_clock::now(),
            interval,
            scheduler
        )
        .map([sensorCopy, baseTemp, noise, rng](int /*i*/) -> Reading {
            std::normal_distribution<double> tempDist(baseTemp, noise);
            std::normal_distribution<double> humDist(45.0, 5.0);
            std::normal_distribution<double> pressDist(1013.0, 2.0);

            return Reading{
                sensorCopy->id,
                std::chrono::system_clock::now(),
                tempDist(*rng),
                humDist(*rng),
                pressDist(*rng)
            };
        });
    }

    [[nodiscard]] const Sensor& sensor() const noexcept { return sensor_; }

private:
    Sensor                     sensor_;
    std::chrono::milliseconds  interval_;
    double                     baseTemp_;
    double                     noise_;
};

} // namespace reactive