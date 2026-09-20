// include/reactive/SensorSource.hpp
#pragma once

#include "Models.hpp"

#include <rxcpp/rx.hpp>

#include <chrono>
#include <memory>
#include <random>

namespace reactive {

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
        auto sensorCopy = std::make_shared<Sensor>(sensor_);
        auto baseTemp   = baseTemp_;
        auto noise      = noise_;
        auto interval   = interval_;

        auto rng = std::make_shared<std::mt19937>(std::random_device{}());

        return rxcpp::observable<>::interval(
            std::chrono::steady_clock::now(),
            interval,
            // CHỈ ĐỊNH SCHEDULER: new_thread
            rxcpp::synchronize_new_thread()
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