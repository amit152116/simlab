#pragma once

#define FMT_HEADER_ONLY

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

class Benchmark {
  public:

    using Clock    = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<double, std::milli>;  // milliseconds

    // RAII scope-based timer
    class Scope {
        Benchmark&        bm;
        Clock::time_point start;

      public:

        Scope(const Scope&)                    = default;
        Scope(Scope&&)                         = delete;
        auto operator=(const Scope&) -> Scope& = delete;
        auto operator=(Scope&&) -> Scope&      = delete;

        explicit Scope(Benchmark& benchmark)
            : bm(benchmark), start(Clock::now()) {}

        ~Scope() {
            auto end = Clock::now();
            bm.addTime(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
    };

    explicit Benchmark(std::string name = "") : name(std::move(name)) {}

    ~Benchmark() {
        report();
    }

    // Start/Stop for manual timing
    void start() {
        startTime = Clock::now();
    }

    void stop() {
        auto end = Clock::now();
        addTime(
            std::chrono::duration<double, std::milli>(end - startTime).count());
    }

    // Benchmark ANY callable: method, function, lambda, functor
    template <typename Callable, typename... Args>
    auto benchmarkCall(Callable&& callable, Args&&... args)
        -> decltype(std::__invoke(std::forward<Callable>(callable),
                                  std::forward<Args>(args)...)) {
        using ResultT = decltype(std::__invoke(std::forward<Callable>(callable),
                                               std::forward<Args>(args)...));

        auto t0 = Clock::now();

        if constexpr (std::is_void_v<ResultT>) {
            std::__invoke(std::forward<Callable>(callable),
                          std::forward<Args>(args)...);
            auto t1 = Clock::now();
            addTime(std::chrono::duration<double, std::milli>(t1 - t0).count());
        } else {
            auto&& result = std::__invoke(std::forward<Callable>(callable),
                                          std::forward<Args>(args)...);
            auto   t1     = Clock::now();
            addTime(std::chrono::duration<double, std::milli>(t1 - t0).count());
            return result;
        }
    }

    // Print benchmark and FPS stats
    void report() const {
        auto headerColor = fmt::color::dark_slate_gray;
        fmt::print(fg(headerColor) | fmt::emphasis::italic,
                   "\n========== Benchmark: '{}' ==========\n", name);
        if (!times.empty()) {
            double sum = 0.0;
            double min = 0.0;
            double max = 0.0;
            for (auto time : times) {
                sum += time;
                min = std::min(time, min);
                max = std::max(time, max);
            }
            double avg = sum / times.size();

            auto color = fmt::color::light_sea_green;
            fmt::print(fg(color), "  Runs       : {}\n", times.size());
            fmt::print(fg(color), "  Avg Time   : {:.3f} ms\n", avg);
            fmt::print(fg(color), "  Min Time   : {:.3f} ms\n", min);
            fmt::print(fg(color), "  Max Time   : {:.3f} ms\n", max);
            fmt::print(fg(color), "  Total Time : {:.3f} ms\n\n", sum);
        }

        if (!fpsPerFrame.empty()) {
            double sumFPS = 0.0;
            double minFPS = fpsPerFrame[0];
            double maxFPS = fpsPerFrame[0];
            for (double fps : fpsPerFrame) {
                sumFPS += fps;
                minFPS = std::min(fps, minFPS);
                maxFPS = std::max(fps, maxFPS);
            }
            double avgFPS = sumFPS / fpsPerFrame.size();

            auto color = fmt::color::purple;
            fmt::print(fg(color), "  Avg FPS    : {:.2f}\n", avgFPS);
            fmt::print(fg(color), "  Min FPS    : {:.2f}\n", minFPS);
            fmt::print(fg(color), "  Max FPS    : {:.2f}\n", maxFPS);
        }
        fmt::print(fg(headerColor) | fmt::emphasis::italic,
                   "====================================\n\n");
    }

  protected:

    // Add a recorded time
    // Add a recorded time and compute FPS based on the previous frame
    void addTime(double ms) {
        times.push_back(ms);

        // Compute FPS between last execution and this one
        if (lastTimestamp.time_since_epoch().count() == 0) {
            // First run, no previous frame
            lastTimestamp = Clock::now();
        } else {
            auto   now = Clock::now();
            double deltaMs =
                std::chrono::duration<double, std::milli>(now - lastTimestamp)
                    .count();
            lastTimestamp = now;
            fpsPerFrame.push_back(1000.0 / deltaMs);
        }
    }

  private:

    std::string         name;
    Clock::time_point   startTime;
    Clock::time_point   lastTimestamp;  // stores timestamp of last execution
    std::vector<double> times;          // measured durations in ms
    std::vector<double> fpsPerFrame;    // FPS between consecutive executions
};
