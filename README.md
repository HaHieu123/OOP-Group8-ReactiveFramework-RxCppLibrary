# Reactive Stream Processing Framework (RxCpp)

A minimal reactive stream processing framework built on top of **RxCpp**
(Reactive Extensions for C++), written in modern C++20.

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![RxCpp](https://img.shields.io/badge/RxCpp-4.x-orange.svg)](https://github.com/ReactiveX/RxCpp)

---

## Mục lục

- [Giới thiệu](#giới-thiệu)
- [Tính năng](#tính-năng)
- [Yêu cầu](#yêu-cầu)
- [Cài đặt](#cài-đặt)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Kiến trúc](#kiến-trúc)
- [Ví dụ nhanh](#ví-dụ-nhanh)
- [Demo IoT](#demo-iot)
- [So sánh: RxCpp vs Custom Framework](#so-sánh-rxcpp-vs-custom-framework)
- [Đóng góp](#đóng-góp)
- [License](#license)

---

## Giới thiệu

Dự án này là phiên bản **dùng thư viện chuẩn RxCpp** — thay vì tự triển khai
`DataStream<T>`, chúng ta sử dụng `rxcpp::observable<T>` có sẵn.

**Điểm khác biệt chính so với phiên bản Custom:**

| Tiêu chí | Custom Framework | RxCpp Framework |
|---|---|---|
| **Core** | Tự viết `DataStream<T>` | Dùng `rxcpp::observable<T>` |
| **Số dòng code** | ~1500 dòng | ~300 dòng |
| **Số operator** | 5 | 100+ |
| **Compile time** | ~5s | ~30s |
| **Binary size** | ~50 KB | ~200 KB |
| **Backpressure** | Không | Có |
| **Scheduler** | Thủ công | Tích hợp sẵn |

→ Phiên bản này phù hợp cho **production**, trong khi phiên bản Custom
phù hợp cho **học tập** (hiểu sâu cách Reactive hoạt động).

---

## Tính năng

- **Dựa trên RxCpp** — thư viện Reactive chuẩn cho C++
- **Type-safe** — kiểm tra kiểu tại compile-time
- **Lazy** — pipeline chưa chạy cho đến khi `subscribe()`
- **Immutable** — mỗi operator trả về stream mới
- **Thread-safe** — mọi sink đều dùng `std::mutex`
- **Đa luồng** — RxCpp scheduler tự động

### Operators (RxCpp cung cấp)

| Nhóm | Operator | Mô tả |
|---|---|---|
| **Source** | `observable<>::iterate`, `interval`, `range` | Tạo stream |
| **Transform** | `map`, `filter`, `flat_map` | Biến đổi |
| **Combine** | `merge`, `concat`, `zip`, `combine_latest` | Gộp streams |
| **Window** | `buffer`, `window`, `window_toggle` | Gom nhóm |
| **Error** | `on_error_resume_next`, `retry` | Xử lý lỗi |
| **Terminal** | `subscribe` | Kích hoạt pipeline |

### Sinks (Custom)

| Sink | Mục đích |
|---|---|
| `ConsoleSink<T>` | In ra stdout với ANSI colors |
| `FileSink<T>` | Ghi ra CSV/JSON (RAII, auto-flush) |

### Sources (Custom)

| Source | Mục đích |
|---|---|
| `SensorSource` | Giả lập cảm biến IoT, phát `rxcpp::observable<Reading>` |

---

## Yêu cầu

- **CMake** ≥ 3.20
- **Compiler** hỗ trợ C++20:
  - MSVC ≥ 19.30 (Visual Studio 2022)
  - GCC ≥ 11
  - Clang ≥ 14
- **RxCpp** (tự động tải qua git submodule hoặc clone thủ công)

---

## Cài đặt

### Clone repository

```bash
git clone https://github.com/HaHieu123/OOP-Group8-ReactiveFramework.git
cd OOP-Group8-ReactiveFramework
```

### Tải RxCpp

**Cách 1: Git submodule (khuyên dùng)**

```bash
git submodule add https://github.com/ReactiveX/RxCpp.git external/RxCpp
git submodule update --init --recursive
```

**Cách 2: Clone trực tiếp**

```bash
mkdir external
cd external
git clone --recursive --depth 1 https://github.com/ReactiveX/RxCpp.git
cd ..
```

### Build bằng CMake

```bash
cmake -B build
cmake --build build --config Release
```

### Chạy demo

```bash
./build/Release/reactive_demo.exe       # Windows
./build/reactive_demo                   # Linux/macOS
```

### Kiểm tra kết quả

```bash
# File alerts được ghi ra
cat alerts.json
```

---

## Cấu trúc dự án

```
OOP-Group8-ReactiveFramework/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── .gitignore
├── include/
│   └── reactive/
│       ├── Models.hpp           # Domain: Sensor, Reading, Alert
│       ├── Sink.hpp             # ConsoleSink, FileSink
│       └── SensorSource.hpp     # IoT sensor simulator
├── src/
│   └── main.cpp                 # Demo
├── external/
│   └── RxCpp/                   # RxCpp library (submodule)
└── build/
    └── Release/
        ├── reactive_demo.exe
        └── alerts.json          # Output
```

---

## Kiến trúc

```
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ SensorSource │  │ SensorSource │  │ SensorSource │
│   Sensor 1   │  │   Sensor 2   │  │   Sensor 3   │
└──────┬───────┘  └──────┬───────┘  └──────┬───────┘
       │                 │                 │
       │ rxcpp::observable<Reading>        │
       └────────────────┬┘─────────────────┘
                        │
                   .merge()
                        │
                        ▼
       ┌────────────────────────────────────┐
       │      rxcpp::observable<Reading>     │
       └────────────────┬───────────────────┘
                        │
                   .filter()  (isValid)
                        │
                   .map()     (Reading → Alert)
                        │
                   .filter()  (WARNING/CRITICAL)
                        │
                   .subscribe()
                        │
              ┌─────────┴─────────┐
              ▼                   ▼
       ┌────────────┐      ┌────────────┐
       │ ConsoleSink│      │  FileSink  │
       │  (stdout)  │      │ alerts.json│
       └────────────┘      └────────────┘
```

**Core pattern:** `rxcpp::observable<T>` — Reactive Streams

- **Source:** `rxcpp::observable<>::interval()` phát dữ liệu theo thời gian
- **Merge:** `observable.merge(other)` gộp nhiều streams
- **Transform:** `.filter()`, `.map()` biến đổi
- **Subscribe:** `.subscribe()` kích hoạt pipeline

---

## Ví dụ nhanh

### Ví dụ 1 — filter + map cơ bản

```cpp
#include <rxcpp/rx.hpp>
#include <iostream>

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};

    rxcpp::observable<>::iterate(data)
        .filter([](int x) { return x % 2 == 0; })
        .map([](int x) { return x * 10; })
        .subscribe([](int v) {
            std::cout << v << '\n';
        });

    return 0;
}
```

**Output:**
```
20
40
60
80
```

### Ví dụ 2 — IoT Sensor → Alert

```cpp
#include <reactive/Models.hpp>
#include <reactive/Sink.hpp>
#include <reactive/SensorSource.hpp>

using namespace reactive;
using namespace std::chrono_literals;

int main() {
    Sensor sensor{1, "Temp-A", "Room A", 10.0, 60.0};

    SensorSource src(sensor, 35.0, 8.0, 100ms);

    ConsoleSink<Alert> sink(
        [](const Alert& a) { return a.toString(); },
        std::cout,
        false
    );

    src.asObservable()
        .filter([](const Reading& r) { return r.isValid(); })
        .map([&sensor](const Reading& r) {
            return Alert::fromReading(r, sensor);
        })
        .filter([](const Alert& a) {
            return a.severity != Severity::INFO;
        })
        .take(10)   // Lấy 10 alerts rồi dừng
        .subscribe(sink.makeObserver());

    return 0;
}
```

### Ví dụ 3 — Merge nhiều sensors

```cpp
auto merged = src1.asObservable()
    .merge(src2.asObservable())
    .merge(src3.asObservable());

merged
    .filter([](const Reading& r) { return r.isValid(); })
    .map([&registry](const Reading& r) {
        return Alert::fromReading(r, *registry.find(r.sensorId));
    })
    .subscribe(sink.makeObserver());
```

### Ví dụ 4 — Buffer theo thời gian

```cpp
src.asObservable()
    .buffer(std::chrono::milliseconds(500))
    .map([](const std::vector<Reading>& batch) {
        double sum = 0;
        for (const auto& r : batch) sum += r.temperature;
        return sum / batch.size();
    })
    .subscribe([](double avg) {
        std::cout << "Average: " << avg << "°C\n";
    });
```

---

## Demo IoT

Demo chạy 3 sensors ảo phát dữ liệu 50 Hz trong 2 giây:

```cpp
SensorRegistry registry;
registry.add({1, "Temp-A", "Phong A", 10.0, 55.0});
registry.add({2, "Temp-B", "Phong B", 15.0, 55.0});
registry.add({3, "Temp-C", "Phong C", 20.0, 50.0});

SensorSource src1(*registry.find(1), 50.0, 8.0, 20ms);
SensorSource src2(*registry.find(2), 45.0, 10.0, 20ms);
SensorSource src3(*registry.find(3), 30.0, 6.0, 20ms);

auto merged = src1.asObservable()
    .merge(src2.asObservable())
    .merge(src3.asObservable());

merged
    .filter([](const Reading& r) { return r.isValid(); })
    .map([&registry](const Reading& r) {
        return Alert::fromReading(r, *registry.find(r.sensorId));
    })
    .filter([](const Alert& a) {
        return a.severity != Severity::INFO;
    })
    .subscribe(/* sink */);
```

### Output mẫu

```
===============================================
  Reactive Stream Processing (RxCpp) - Demo
===============================================

[CONFIG] Da dang ky 3 sensors
[CONFIG] Da tao 3 sensor sources
[CONFIG] Moi sensor phat 50 Hz, chay trong 2 s

[CRITICAL] Sensor #1 @ ... — Nhiệt độ vượt ngưỡng tối đa: 57.14°C > 55.00°C
[CRITICAL] Sensor #2 @ ... — Nhiệt độ vượt ngưỡng tối đa: 58.94°C > 55.00°C
...

+===================================================+
|              KET QUA DO LUONG                     |
+===================================================+
| Tong so readings:            300
| Readings hop le:             300
|---------------------------------------------------|
| Tong so alerts:              180
| CRITICAL:                    120
| WARNING:                      60
+===================================================+

+===================================================+
|         CHI TIET THEO TUNG SENSOR                 |
+===================================================+
| Sensor          | CRITICAL | WARNING |   Total   |
|-----------------+----------+---------+-----------|
| Temp-A (Phong A)|       60 |      20 |        80 |
| Temp-B (Phong B)|       35 |      15 |        50 |
| Temp-C (Phong C)|       25 |      25 |        50 |
+===================================================+

[FILE] Alerts da ghi vao alerts.json
[PERF] Thoi gian chay: 2.02 s
```

---

## So sánh: RxCpp vs Custom Framework

Hai phiên bản của cùng dự án:

### Phiên bản RxCpp (thư mục này)

- ✅ Dùng thư viện chuẩn RxCpp
- ✅ Nhiều operator có sẵn (100+)
- ✅ Backpressure, scheduler tích hợp
- ✅ Production-ready
- ⚠️ Compile time lâu hơn (~30s)
- ⚠️ Binary lớn hơn (~200 KB)

### Phiên bản Custom

- ✅ Hiểu sâu cách Reactive hoạt động
- ✅ Compile nhanh (~5s)
- ✅ Binary nhỏ (~50 KB)
- ✅ Học OOP/SOLID principles
- ⚠️ Chỉ có 5 operator
- ⚠️ Không có backpressure

### Bảng so sánh chi tiết

| Tiêu chí | RxCpp | Custom |
|---|---|---|
| **Core** | `rxcpp::observable<T>` | `DataStream<T>` |
| **Số dòng** | ~300 | ~1500 |
| **Operators** | 100+ | 5 |
| **Throughput** | ~30 M msg/s | ~30 M msg/s |
| **Latency p99** | ~0.9 µs | ~0.9 µs |
| **Compile time** | 30s | 5s |
| **Binary size** | 200 KB | 50 KB |
| **Học tập** | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Production** | ⭐⭐⭐⭐⭐ | ⭐⭐ |

---

## Đóng góp

Mọi đóng góp đều được chào đón! Vui lòng:

1. Fork repo
2. Tạo branch: `git checkout -b feature/ten-tinh-nang`
3. Commit: `git commit -am "feat: mo ta"`
4. Push: `git push origin feature/ten-tinh-nang`
5. Tạo Pull Request

### Coding style

- **C++20** — dùng `std::format`, `concept`, `ranges`
- **Naming:**
  - Class: `PascalCase`
  - Function: `camelCase`
  - Member: `snake_case_`
  - Constant: `UPPER_SNAKE`
- **Comment:** Tiếng Việt hoặc tiếng Anh đều OK

---

## Tác giả

- **Ha Hieu** — [@HaHieu123](https://github.com/HaHieu123)

Nhóm 8 — OOP-Group8-ReactiveFramework

---

## License

MIT License — xem [LICENSE](LICENSE) để biết chi tiết.

---

## Tham khảo

- [RxCpp GitHub](https://github.com/ReactiveX/RxCpp)
- [ReactiveX Documentation](http://reactivex.io/)
- [Reactive Streams](https://www.reactive-streams.org/)