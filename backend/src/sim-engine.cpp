#include "sim-engine.h"
#include <random>
#include <thread>
#include <iostream>

SimulationEngine::SimulationEngine()
{
    std::cout << "SimulationEngine created" << std::endl;
}

SimulationEngine::~SimulationEngine() {
    stop();
}

void SimulationEngine::start() {
    if (_running) {
        return;
    }

    std::cout << "Starting simulation thread..." << std::endl;
    _running = true;
    _sim_thread = std::make_unique<std::thread>(&SimulationEngine::runLoop, this);
}

void SimulationEngine::stop() {
    if (!_running) {
        return;
    }

    std::cout << "Stopping simulation engine..." << std::endl;
    _running = false;
    _cv.notify_all();

    if (_sim_thread && _sim_thread->joinable()) {
        _sim_thread->join();
    }
    std::cout << "Simulation engine stopped." << std::endl;
}

void SimulationEngine::togglePause() {
    _paused = !_paused;
}

void SimulationEngine::runLoop() {
    try {
        constexpr std::chrono::milliseconds UPDATE_INTERVAL(500);
        _last_update_time = std::chrono::steady_clock::now();

        while (_running) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _last_update_time);

            if (elapsed >= UPDATE_INTERVAL) {
                if (!_paused) {
                    update();
                }
                _last_update_time = now;
            }

            std::unique_lock<std::mutex> lock(_cv_mutex);
            _cv.wait_for(lock, std::chrono::milliseconds(50), [this] {
                return !_running;
            });
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\n!!! SIMULATION THREAD CRASHED !!!\n"
            << e.what() << std::endl;
        _running = false;
    }
    catch (...) {
        std::cerr << "\n!!! UNKNOWN EXCEPTION IN SIMULATION THREAD !!!" << std::endl;
        _running = false;
    }
}

void SimulationEngine::update() {
    std::lock_guard<std::mutex> lock(_data_mutex);

    static int counter = 0;
    if (counter % 20 == 0) {
        addTarget();
        counter = 0;
    }
    counter++;

    for (auto& target : _targets) {
        target.move();
    }
}

void SimulationEngine::addTarget() {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis_dist(0, 200);
        std::uniform_real_distribution<> dis_angle(0, 2 * EIGEN_PI);

        Target new_target(
            static_cast<int>(_targets.size() + 1),
            dis_dist(gen),
            dis_angle(gen),
            dis_angle(gen),
            generateRandomColor()
        );

        _targets.push_back(new_target);
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding target: " << e.what() << std::endl;
    }
}

std::vector<Target> SimulationEngine::getTargets() const {
    std::lock_guard<std::mutex> lock(_data_mutex);
    return _targets;
}

bool SimulationEngine::isRunning() const {
    return _running;
}

bool SimulationEngine::isPaused() const {
    return _paused;
}