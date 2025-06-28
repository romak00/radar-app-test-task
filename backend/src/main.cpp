#include "network-server.h"
#include <iostream>
#include <thread>

int main() {
    try {
        asio::io_context io_ctx;
        SimulationEngine engine;
        NetworkServer server(io_ctx, 5555, engine);

        asio::signal_set signals(io_ctx, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::cout << "\nSignal received. Shutting down..." << std::endl;
            server.stop();
            engine.stop();
            io_ctx.stop();
            });

        engine.start();
        server.start();

        std::cout << "Server running. Press Ctrl+C or send EXIT command to stop." << std::endl;
        io_ctx.run();

        std::cout << "Server stopped." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n!!! CRITICAL ERROR !!!\n" << e.what() << std::endl;
        return 1;
    }
}