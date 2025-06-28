#pragma once

#include <memory>
#include <vector>
#include <asio.hpp>
#include "sim-engine.h"

class NetworkServer {
public:
    NetworkServer(asio::io_context& io, int port, SimulationEngine& engine);
    ~NetworkServer();

    void start();
    void stop();
    void broadcastData();

private:
    void startAccept();
    void startBroadcast();
    void readCommands(asio::ip::tcp::socket& socket);
    void handleCommand(const std::string& command);

    template<typename T>
    static void appendToBuffer(std::vector<uint8_t>& buffer, const T& value) {
        const auto* data = reinterpret_cast<const uint8_t*>(&value);
        buffer.insert(buffer.end(), data, data + sizeof(T));
    }

    SimulationEngine& _sim_eng;
    std::vector<asio::ip::tcp::socket> _clients;
    asio::io_context& _io_ctx;
    std::unique_ptr<asio::steady_timer> _broadcast_timer;
    asio::ip::tcp::acceptor _acceptor;
    std::mutex _clients_mutex;

    std::atomic<bool> _stopped{ false };
};