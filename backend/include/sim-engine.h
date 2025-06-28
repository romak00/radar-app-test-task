#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <condition_variable>
#include "target.h"

class SimulationEngine {
public:
    SimulationEngine();
    ~SimulationEngine();

    void start();
    void stop();
    void togglePause();

    void update();

    std::vector<Target> getTargets() const;
    bool isRunning() const;
    bool isPaused() const;

private:
    void addTarget();
    void runLoop();

    std::vector<Target> _targets;
    std::unique_ptr<std::thread> _sim_thread;
    mutable std::mutex _data_mutex;
    std::atomic<bool> _running{ false };
    std::atomic<bool> _paused{ false };
    std::chrono::steady_clock::time_point _last_update_time;
    std::condition_variable _cv;
    std::mutex _cv_mutex;
};