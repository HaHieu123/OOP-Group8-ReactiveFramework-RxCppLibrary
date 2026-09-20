// src/main.cpp
// ============================================================================
// DEMO: Hệ thống giám sát môi trường IoT dùng RxCpp
// ============================================================================
#include <reactive/Models.hpp>
#include <reactive/Sink.hpp>
#include <reactive/SensorSource.hpp>

#include <rxcpp/rx.hpp>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace reactive;
using namespace std::chrono_literals;

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    auto t0 = std::chrono::steady_clock::now();
    auto logElapsed = [&t0]() {          // <-- ĐÃ ĐỔI TÊN
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0
        ).count();
    };

    std::cerr << "[" << logElapsed() << " ms] START\n";

    std::cout << "===============================================\n";
    std::cout << "  Reactive Stream Processing (RxCpp) - Demo\n";
    std::cout << "===============================================\n\n";

    // 1. Sensor metadata
    SensorRegistry registry;
    registry.add({1, "Temp-A", "Phong A", 10.0, 55.0});
    registry.add({2, "Temp-B", "Phong B", 15.0, 55.0});
    registry.add({3, "Temp-C", "Phong C", 20.0, 50.0});

    std::cout << "[CONFIG] Da dang ky " << registry.size() << " sensors\n";
    std::cerr << "[" << logElapsed() << " ms] Registry OK\n";

    auto s1 = registry.find(1);
    auto s2 = registry.find(2);
    auto s3 = registry.find(3);

    if (!s1 || !s2 || !s3) {
        std::cerr << "[FATAL] Khong tim thay sensor metadata\n";
        return 1;
    }

    constexpr auto EMIT_INTERVAL = 20ms;

    SensorSource src1(*s1, 50.0, 8.0, EMIT_INTERVAL);
    SensorSource src2(*s2, 45.0, 10.0, EMIT_INTERVAL);
    SensorSource src3(*s3, 30.0, 6.0, EMIT_INTERVAL);

    std::cout << "[CONFIG] Da tao 3 sensor sources\n";
    std::cout << "[CONFIG] Moi sensor phat 50 Hz, chay trong 2 s\n";
    std::cout << "[CONFIG] Chi in moi 200 alerts CRITICAL\n\n";
    std::cerr << "[" << logElapsed() << " ms] Sources OK\n";

    // 2. Merge
    auto merged = src1.asObservable()
        .merge(src2.asObservable())
        .merge(src3.asObservable());
    std::cerr << "[" << logElapsed() << " ms] Merge OK\n";

    // 3. Sinks
    ConsoleSink<Alert> consoleSink(
        [](const Alert& a) { return a.toString(); },
        std::cout,
        true
    );

    FileSink<Alert> fileSink(
        "alerts.json",
        FileSink<Alert>::Format::JSON
    );

    std::cerr << "[" << logElapsed() << " ms] Sinks OK\n";

    // 4. Pipeline
    std::atomic<std::uint64_t> totalReadings{0};
    std::atomic<std::uint64_t> validReadings{0};
    std::atomic<std::uint64_t> alertCount{0};
    std::atomic<std::uint64_t> criticalCount{0};
    std::atomic<std::uint64_t> warningCount{0};
    std::atomic<std::uint64_t> consolePrintCount{0};

    std::atomic<std::uint64_t> criticalBySensor[4] = {};
    std::atomic<std::uint64_t> warningBySensor[4] = {};

    auto consoleObserver = consoleSink.makeObserver();
    auto fileObserver    = fileSink.makeObserver();

    std::cerr << "[" << logElapsed() << " ms] BEFORE subscribe\n";

    auto subscription = merged
        .filter([&totalReadings](const Reading& r) {
            ++totalReadings;
            return r.isValid();
        })
        .filter([&registry, &validReadings](const Reading& r) {
            const bool ok = registry.find(r.sensorId).has_value();
            if (ok) ++validReadings;
            return ok;
        })
        .map([&registry](const Reading& r) -> Alert {
            auto sensor = registry.find(r.sensorId);
            return Alert::fromReading(r, *sensor);
        })
        .filter([](const Alert& a) {
            return a.severity == Severity::WARNING
                || a.severity == Severity::CRITICAL;
        })
        .subscribe(
            [&](const Alert& a) {
                ++alertCount;

                if (a.severity == Severity::CRITICAL) {
                    ++criticalCount;
                    if (a.sensorId >= 1 && a.sensorId <= 3) {
                        ++criticalBySensor[a.sensorId];
                    }
                    if (criticalCount % 200 == 0) {
                        ++consolePrintCount;
                        consoleObserver.on_next(a);
                    }
                } else {
                    ++warningCount;
                    if (a.sensorId >= 1 && a.sensorId <= 3) {
                        ++warningBySensor[a.sensorId];
                    }
                }

                fileObserver.on_next(a);
            },
            [](std::exception_ptr e) {
                try {
                    if (e) std::rethrow_exception(e);
                } catch (const std::exception& ex) {
                    std::cerr << "[ERROR] " << ex.what() << '\n';
                }
            },
            []() {
                std::cerr << "[DONE] Pipeline complete\n";
            }
        );

    std::cerr << "[" << logElapsed() << " ms] AFTER subscribe\n";
    std::cerr << "[" << logElapsed() << " ms] BEFORE sleep 2s\n";

    std::this_thread::sleep_for(2s);

    std::cerr << "[" << logElapsed() << " ms] AFTER sleep 2s\n";
    std::cerr << "[" << logElapsed() << " ms] BEFORE unsubscribe\n";

    subscription.unsubscribe();

    std::cerr << "[" << logElapsed() << " ms] AFTER unsubscribe\n";

    // 5. In kết quả
    std::cout << "\n+===================================================+\n";
    std::cout << "|              KET QUA DO LUONG                     |\n";
    std::cout << "+===================================================+\n";
    std::cout << "| Tong so readings:       " << std::setw(8) << totalReadings.load() << "\n";
    std::cout << "| Readings hop le:        " << std::setw(8) << validReadings.load() << "\n";
    std::cout << "|---------------------------------------------------|\n";
    std::cout << "| Tong so alerts:         " << std::setw(8) << alertCount.load() << "\n";
    std::cout << "| CRITICAL:               " << std::setw(8) << criticalCount.load() << "\n";
    std::cout << "| WARNING:                " << std::setw(8) << warningCount.load() << "\n";
    std::cout << "+===================================================+\n";

    std::cout << "\n+===================================================+\n";
    std::cout << "|         CHI TIET THEO TUNG SENSOR                 |\n";
    std::cout << "+===================================================+\n";
    std::cout << "| Sensor          | CRITICAL | WARNING |   Total   |\n";
    std::cout << "|-----------------+----------+---------+-----------|\n";

    const char* sensorNames[4] = {"", "Temp-A (Phong A)", "Temp-B (Phong B)", "Temp-C (Phong C)"};
    for (int i = 1; i <= 3; ++i) {
        const auto c = criticalBySensor[i].load();
        const auto w = warningBySensor[i].load();
        std::cout << "| " << std::setw(15) << std::left << sensorNames[i]
                  << " | " << std::setw(8) << std::right << c
                  << " | " << std::setw(7) << w
                  << " | " << std::setw(9) << (c + w) << " |\n";
    }
    std::cout << "+===================================================+\n";

    auto programEnd = std::chrono::steady_clock::now();
    const double totalTime = std::chrono::duration<double>(programEnd - t0).count();

    std::cout << "\n[FILE] Alerts da ghi vao alerts.json\n";
    std::cout << "[CONSOLE] Da in " << consolePrintCount.load() << " alerts\n";
    std::cout << "[PERF] Thoi gian chay: " << std::fixed << std::setprecision(2) << totalTime << " s\n";
    std::cout << "\nDemo hoan tat!\n";

    std::cout.flush();
    std::cerr << "[" << logElapsed() << " ms] EXIT\n";
    std::cerr.flush();

#ifdef _WIN32
    TerminateProcess(GetCurrentProcess(), 0);
#else
    std::exit(0);
#endif
}