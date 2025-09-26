#include "simlab/PhysicsManager.hpp"

namespace simlab {

    PhysicsManager::PhysicsManager() : m_fixedDeltaTime(1.0F / m_targetFPS) {}

    PhysicsManager::~PhysicsManager() {
        stop();
    }

    void PhysicsManager::setPhysicsFunction(PhysicsFunction physicsFunc) {
        std::scoped_lock lock(m_controlMutex);
        m_physicsFunction = physicsFunc;
    }

    void PhysicsManager::setPhysicsFunction(
        std::function<void(float)> simplePhysicsFunc) {
        std::scoped_lock lock(m_controlMutex);
        m_physicsFunction = [simplePhysicsFunc](float       dt,
                                                std::mutex& dataMutex) -> void {
            std::scoped_lock dataLock(dataMutex);
            simplePhysicsFunc(dt);
        };
    }

    void PhysicsManager::setPrePhysicsCallback(PhysicsCallback callback) {
        std::scoped_lock lock(m_controlMutex);
        m_prePhysicsCallback = callback;
    }

    void PhysicsManager::setPrePhysicsCallback(
        std::function<void()>& callback) {
        std::scoped_lock lock(m_controlMutex);

        m_prePhysicsCallback = [callback](std::mutex& dataMutex) -> void {
            std::scoped_lock dataLock(dataMutex);
            callback();
        };
    }

    void PhysicsManager::setPostPhysicsCallback(PhysicsCallback callback) {
        std::scoped_lock lock(m_controlMutex);
        m_postPhysicsCallback = callback;
    }

    void PhysicsManager::setPostPhysicsCallback(
        std::function<void()>& callback) {
        std::scoped_lock lock(m_controlMutex);

        m_postPhysicsCallback = [callback](std::mutex& dataMutex) -> void {
            std::scoped_lock dataLock(dataMutex);
            callback();
        };
    }

    auto PhysicsManager::start() -> bool {
        std::scoped_lock lock(m_controlMutex);

        if (m_state != ThreadState::STOPPED || !m_physicsFunction) {
            return false;
        }

        m_state          = ThreadState::RUNNING;
        m_lastUpdateTime = std::chrono::steady_clock::now();
        m_accumulator    = 0.0F;
        m_totalUpdates   = 0;

        m_physicsThread = std::thread(&PhysicsManager::physicsLoop, this);
        return true;
    }

    void PhysicsManager::stop() {
        {
            std::scoped_lock lock(m_controlMutex);
            if (m_state == ThreadState::STOPPED)
                return;
            m_state = ThreadState::STOPPED;
        }

        m_pauseCondition.notify_all();

        if (m_physicsThread.joinable()) {
            m_physicsThread.join();
        }
    }

    void PhysicsManager::pause() {
        std::scoped_lock lock(m_controlMutex);
        if (m_state == ThreadState::RUNNING) {
            m_state = ThreadState::PAUSED;
        }
    }

    void PhysicsManager::resume() {
        {
            std::scoped_lock lock(m_controlMutex);
            if (m_state == ThreadState::PAUSED) {
                m_state          = ThreadState::RUNNING;
                m_lastUpdateTime = std::chrono::steady_clock::now();
            }
        }
        m_pauseCondition.notify_all();
    }

    void PhysicsManager::waitForUpdates(
        int updateCount, std::chrono::milliseconds timeout) const {
        uint64_t startUpdates = getTotalUpdates();
        auto     startTime    = std::chrono::steady_clock::now();

        while (getTotalUpdates() - startUpdates <
               static_cast<uint64_t>(updateCount)) {
            if (std::chrono::steady_clock::now() - startTime > timeout) {
                break;
            }
            std::this_thread::sleep_for(1ms);
        }
    }

    template <typename Func>
    auto PhysicsManager::executeOnce(Func&& func)
        -> std::future<decltype(func())> {
        using ReturnT = decltype(func());
        auto promise  = std::make_shared<std::promise<ReturnT>>();
        auto future   = promise->get_future();

        auto task = [func = std::forward<Func>(func),
                     promise]() mutable -> auto {
            if constexpr (std::is_void_v<ReturnT>) {
                func();
                promise->set_value();
            } else {
                promise->set_value(func());
            }
        };

        {
            std::scoped_lock lock(m_taskMutex);
            m_taskQueue.push(std::move(task));
        }

        return future;
    }

    auto PhysicsManager::getPerformanceStats() const -> PerformanceStats {
        return {getActualFPS(), getTargetFPS(), getTotalUpdates(),
                getState(),     m_maxDeltaTime, m_maxSubSteps};
    }

    void PhysicsManager::physicsLoop() {
        Benchmark bm("Physics Loop");

        auto     lastFPSTime  = std::chrono::steady_clock::now();
        uint64_t framesForFPS = 0;

        while (m_state != ThreadState::STOPPED) {
            Benchmark::Scope scope(bm);

            // Handle pause
            {
                std::unique_lock<std::mutex> pauseLock(m_controlMutex);
                m_pauseCondition.wait(pauseLock, [this]() -> bool {
                    return m_state == ThreadState::RUNNING ||
                           m_state == ThreadState::STOPPED;
                });
                if (m_state == ThreadState::STOPPED) {
                    break;
                }
            }

            // ---- Frame Timing ----
            auto  currentTime = std::chrono::steady_clock::now();
            float frameTime =
                std::chrono::duration<float>(currentTime - m_lastUpdateTime)
                    .count();
            m_lastUpdateTime = currentTime;

            // Cap large delta times (avoid spiral of death if lag spike)
            frameTime = std::min(frameTime, m_maxDeltaTime);

            // ---- Physics Update ----
            if (m_useFixedTimeStep) {
                fixedTimeStepUpdate(frameTime);
            } else {
                variableTimeStepUpdate(frameTime);
            }

            // Update FPS counter
            framesForFPS++;
            auto fpsDuration =
                std::chrono::duration<float>(currentTime - lastFPSTime).count();
            if (fpsDuration >= 1.0F) {
                m_actualFPS  = static_cast<float>(framesForFPS) / fpsDuration;
                framesForFPS = 0;
                lastFPSTime  = currentTime;
            }

            std::this_thread::sleep_for(1ms);
        }
    }

    void PhysicsManager::fixedTimeStepUpdate(float frameTime) {
        m_accumulator += frameTime;

        int subSteps = 0;
        while (m_accumulator >= m_fixedDeltaTime && subSteps < m_maxSubSteps) {
            // executePhysicsUpdate(m_fixedDeltaTime);
            bm.benchmarkCall(&PhysicsManager::executePhysicsUpdate, *this,
                             m_fixedDeltaTime);

            m_accumulator -= m_fixedDeltaTime;
            subSteps++;
        }

        // Keep small leftover for smooth simulation (don’t reset to 0!)
        m_accumulator = std::min(m_accumulator, m_fixedDeltaTime);
    }

    void PhysicsManager::variableTimeStepUpdate(float frameTime) {
        // executePhysicsUpdate(frameTime);
        bm.benchmarkCall(&PhysicsManager::executePhysicsUpdate, *this,
                         frameTime);
    }

    void PhysicsManager::executePhysicsUpdate(float deltaTime) {
        // Execute queued tasks safely
        {
            std::scoped_lock lock(m_taskMutex);
            if (!m_taskQueue.empty()) {
                auto task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
                task();
            }
        }

        // Pre-physics callback
        if (m_prePhysicsCallback) {
            m_prePhysicsCallback(m_sharedDataMutex);
        }

        // Main physics function - PASSES MUTEX REFERENCE
        if (m_physicsFunction) {
            m_physicsFunction(deltaTime, m_sharedDataMutex);
        }

        // Post-physics callback
        if (m_postPhysicsCallback) {
            m_postPhysicsCallback(m_sharedDataMutex);
        }

        m_totalUpdates++;
    }
}  // namespace simlab
