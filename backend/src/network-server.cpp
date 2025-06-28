#include "network-server.h"
#include <iostream>
#include <thread>
#include <csignal>

NetworkServer::NetworkServer(
    asio::io_context& io,
    int port,
    SimulationEngine& engine
) :
    _io_ctx(io),
    _acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    _sim_eng(engine)
{
    if (!_acceptor.is_open()) {
        throw std::runtime_error("Failed to open acceptor on port " + std::to_string(port));
    }
    std::cout << "Server listening on port " << port << std::endl;

}

NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::start() {
    startAccept();

    if (!_broadcast_timer) {
        _broadcast_timer = std::make_unique<asio::steady_timer>(_io_ctx);
    }

    startBroadcast();
}

void NetworkServer::startBroadcast() {
    _broadcast_timer->expires_after(std::chrono::milliseconds(500));
    _broadcast_timer->async_wait(
        [this](const asio::error_code& ec) {
            if (ec) {
                if (ec != asio::error::operation_aborted) {
                    std::cerr << "Broadcast timer error: " << ec.message() << std::endl;
                }
                return;
            }

            try {
                broadcastData();
            }
            catch (const std::exception& e) {
                std::cerr << "Broadcast error: " << e.what() << std::endl;
            }
            startBroadcast();
        }
    );
}

void NetworkServer::stop() {
    if (_stopped) {
        return;
    }
    _stopped = true;

    if (_broadcast_timer) {
        _broadcast_timer->cancel();
    }

    std::lock_guard<std::mutex> lock(_clients_mutex);
    for (auto& socket : _clients) {
        if (socket.is_open()) {
            asio::error_code ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket.close(ec);
        }
    }
    _clients.clear();
}

void NetworkServer::startAccept() {
    _acceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec) {
            std::lock_guard<std::mutex> lock(_clients_mutex);
            _clients.push_back(std::move(socket));

            readCommands(_clients.back());
        }
        else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }
        startAccept();
    });
}

void NetworkServer::readCommands(asio::ip::tcp::socket& socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024);

    socket.async_read_some(asio::buffer(*buffer),
        [this, buffer, &socket](asio::error_code ec, size_t length) {
            if (!ec) {
                std::string command(buffer->begin(), buffer->begin() + length);
                handleCommand(command);

                readCommands(socket);
            }
            else {
                std::lock_guard<std::mutex> lock(_clients_mutex);
                auto it = std::find_if(_clients.begin(), _clients.end(),
                    [&socket](asio::ip::tcp::socket& s) {
                        return &s == &socket;
                    });

                if (it != _clients.end()) {
                    it->close();
                    _clients.erase(it);
                }
            }
        });
}

void NetworkServer::handleCommand(const std::string& command) {
    if (command == "PAUSE") {
        if (!_sim_eng.isPaused()) {
            _sim_eng.togglePause();
        }
    }
    else if (command == "RESUME") {
        if (_sim_eng.isPaused()) {
            _sim_eng.togglePause();
        }
    }
    else if (command == "EXIT") {
        std::cout << "Exit command received from client" << '\n';

        asio::post(_io_ctx, [this] {
            this->stop();
            _io_ctx.stop();
            });
    }
}

void NetworkServer::broadcastData() {
    if (_clients.empty()) {
        return;
    }

    auto targets = _sim_eng.getTargets();

    std::vector<uint8_t> buffer;

    uint32_t magic = 0xABCDEF01;
    NetworkServer::appendToBuffer(buffer, magic);

    uint16_t count = targets.size();
    NetworkServer::appendToBuffer(buffer, count);

    for (const auto& t : targets) {
        NetworkServer::appendToBuffer(buffer, t.id);
        NetworkServer::appendToBuffer(buffer, t.distance);
        NetworkServer::appendToBuffer(buffer, t.angle);
        NetworkServer::appendToBuffer(buffer, t.direction);

        NetworkServer::appendToBuffer(buffer, t.color.x());
        NetworkServer::appendToBuffer(buffer, t.color.y());
        NetworkServer::appendToBuffer(buffer, t.color.z());

        uint8_t trail_size = static_cast<uint8_t>(t.trail.size());
        NetworkServer::appendToBuffer(buffer, trail_size);

        for (const auto& point : t.trail) {
            NetworkServer::appendToBuffer(buffer, point.x());
            NetworkServer::appendToBuffer(buffer, point.y());
        }
    }

    std::lock_guard<std::mutex> lock(_clients_mutex);
    for (auto& socket : _clients) {
        if (socket.is_open()) {
            asio::error_code ec;
            socket.write_some(asio::buffer(buffer), ec);
            if (ec) {
                socket.close();
            }
        }
    }
}