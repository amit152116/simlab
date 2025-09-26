#pragma once

#include "simlab/Benchmark.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <queue>
#include <thread>

namespace simlab {
    using namespace std::chrono_literals;

    /**
     * @brief A flexible threaded physics manager that can execute any physics
     * function You provide the physics logic, it handles the threading
     */

    class PhysicsManager {
      public:

        // Physics function that receives both deltaTime and a mutex reference
        using PhysicsFunction =
            std::function<void(float deltaTime, std::mutex& dataMutex)>;
        using PhysicsCallback = std::function<void(std::mutex& dataMutex)>;

        enum class ThreadState : uint8_t { STOPPED, RUNNING, PAUSED };

        PhysicsManager(const PhysicsManager&)                    = delete;
        PhysicsManager(PhysicsManager&&)                         = delete;
        auto operator=(const PhysicsManager&) -> PhysicsManager& = delete;
        auto operator=(PhysicsManager&&) -> PhysicsManager&      = delete;

        PhysicsManager();

        ~PhysicsManager();

      private:

        // Thread management
        std::thread                       m_physicsThread;
        std::atomic<ThreadState>          m_state{ThreadState::STOPPED};
        std::mutex                        m_controlMutex;
        std::condition_variable           m_pauseCondition;
        std::queue<std::function<void()>> m_taskQueue;
        std::mutex                        m_taskMutex;

        // SHARED DATA MUTEX - This is the key!
        mutable std::mutex m_sharedDataMutex;

        // Physics function and callbacks
        PhysicsFunction m_physicsFunction;
        PhysicsCallback m_prePhysicsCallback;
        PhysicsCallback m_postPhysicsCallback;

        // Timing control
        float m_targetFPS = 120.0F;
        float m_fixedDeltaTime;
        bool  m_useFixedTimeStep = true;
        float m_maxDeltaTime     = 1.0F / 30.0F;
        int   m_maxSubSteps      = 4;

        // Timing state
        std::chrono::steady_clock::time_point m_lastUpdateTime;
        float                                 m_accumulator = 0.0F;

        // Statistics
        std::atomic<float>    m_actualFPS{0.0F};
        std::atomic<uint64_t> m_totalUpdates{0};

      public:

        // ========== PHYSICS FUNCTION SETUP ==========

        /**
         * @brief Set physics function that receives deltaTime AND mutex
         * reference
         */
        void setPhysicsFunction(PhysicsFunction physicsFunc);

        /**
         * @brief Set physics function (simple version - auto-locks internally)
         */
        void setPhysicsFunction(std::function<void(float)> simplePhysicsFunc);

        void setPrePhysicsCallback(PhysicsCallback callback);

        void setPrePhysicsCallback(std::function<void()>& callback);

        void setPostPhysicsCallback(PhysicsCallback callback);

        void setPostPhysicsCallback(std::function<void()>& callback);

        // ========== SHARED DATA ACCESS METHODS ==========

        /**
         * @brief Get reference to the shared data mutex
         * Use this in your main loop for thread-safe access
         */
        auto getDataMutex() const -> std::mutex& {
            return m_sharedDataMutex;
        }

        /**
         * @brief Execute a function with automatic data mutex locking
         * Use this in main loop for safe data access
         */
        template <typename Func>
        auto withDataLock(Func&& func) const -> decltype(func()) {
            std::scoped_lock lock(m_sharedDataMutex);
            return func();
        }

        // ========== THREAD CONTROL METHODS ==========

        auto start() -> bool;

        void stop();

        void pause();

        void resume();

        // ========== CONFIGURATION METHODS ==========

        void setTargetFPS(float fps) {
            std::scoped_lock lock(m_controlMutex);
            m_targetFPS      = fps;
            m_fixedDeltaTime = 1.0F / fps;
        }

        void setFixedTimeStep(bool enabled) {
            std::scoped_lock lock(m_controlMutex);
            m_useFixedTimeStep = enabled;
        }

        void setMaxDeltaTime(float maxDt) {
            std::scoped_lock lock(m_controlMutex);
            m_maxDeltaTime = maxDt;
        }

        void setMaxSubSteps(int maxSteps) {
            std::scoped_lock lock(m_controlMutex);
            m_maxSubSteps = maxSteps;
        }

        // ========== STATE QUERY METHODS ==========

        auto getState() const -> ThreadState {
            return m_state.load();
        }

        auto isRunning() const -> bool {
            return m_state == ThreadState::RUNNING;
        }

        auto isPaused() const -> bool {
            return m_state == ThreadState::PAUSED;
        }

        auto isStopped() const -> bool {
            return m_state == ThreadState::STOPPED;
        }

        auto getActualFPS() const -> float {
            return m_actualFPS.load();
        }

        auto getTargetFPS() const -> float {
            return m_targetFPS;
        }

        auto getTotalUpdates() const -> uint64_t {
            return m_totalUpdates.load();
        }

        // ========== UTILITY METHODS FOR MAIN LOOP ==========

        /**
         * @brief Wait for the physics thread to complete a specific number of
         * updates Useful for synchronizing main thread with physics
         */
        void waitForUpdates(int                       updateCount,
                            std::chrono::milliseconds timeout = 1000ms) const;

        template <typename Func>
        auto executeOnce(Func&& func) -> std::future<decltype(func())>;

        /**
         * @brief Get performance statistics
         */
        struct PerformanceStats {
            float       actualFPS;
            float       targetFPS;
            uint64_t    totalUpdates;
            ThreadState state;
            float       maxDeltaTime;
            int         maxSubSteps;
        };

        auto getPerformanceStats() const -> PerformanceStats;

      private:

        void physicsLoop();

        void fixedTimeStepUpdate(float frameTime);

        Benchmark bm = Benchmark(":executePhysicsUpdate");

        void variableTimeStepUpdate(float frameTime);

        void executePhysicsUpdate(float deltaTime);
    };

}  // namespace simlab
