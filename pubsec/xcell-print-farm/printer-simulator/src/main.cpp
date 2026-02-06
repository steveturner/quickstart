#include <Ditto.h>
#include "PrinterSimulator.h"
#include "TelemetryWriter.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

using namespace ditto;
using namespace std::chrono;

// Environment variable helpers
std::string getEnvOrThrow(const char* name) {
    const char* value = std::getenv(name);
    if (!value) {
        throw std::runtime_error(
            std::string("Missing required environment variable: ") + name
        );
    }
    return std::string(value);
}

std::string getEnvOrDefault(const char* name, const std::string& defaultVal) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultVal;
}

double getEnvDoubleOrDefault(const char* name, double defaultVal) {
    const char* value = std::getenv(name);
    if (!value) return defaultVal;
    try {
        return std::stod(value);
    } catch (...) {
        return defaultVal;
    }
}

int main() {
    try {
        // Read Ditto environment variables
        std::string app_id = getEnvOrThrow("DITTO_APP_ID");
        std::string token = getEnvOrThrow("DITTO_PLAYGROUND_TOKEN");
        std::string auth_url = getEnvOrThrow("DITTO_AUTH_URL");
        std::string websocket_url = getEnvOrThrow("DITTO_WEBSOCKET_URL");

        // Read printer configuration
        std::string printer_id = getEnvOrDefault("PRINTER_ID", "printer-001");

        // Read location configuration
        Location location;
        location.name = getEnvOrDefault("LOCATION_NAME", "Unknown Location");
        location.site_code = getEnvOrDefault("LOCATION_SITE_CODE", "UNKNOWN");
        location.latitude = getEnvDoubleOrDefault("LOCATION_LATITUDE", 0.0);
        location.longitude = getEnvDoubleOrDefault("LOCATION_LONGITUDE", 0.0);

        std::cout << "========================================" << std::endl;
        std::cout << "xCell Printer Simulator" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Printer ID: " << printer_id << std::endl;
        std::cout << "Location: " << location.name << " (" << location.site_code << ")" << std::endl;
        std::cout << "Coordinates: " << location.latitude << ", " << location.longitude << std::endl;
        std::cout << "========================================" << std::endl;

        // Create Ditto identity (Online Playground for development)
        // Use custom auth URL from environment and disable default cloud sync
        // to rely on WebSocket transport (matching web app configuration)
        auto identity = Identity::OnlinePlayground(
            app_id,
            token,
            false,   // disable default cloud sync
            auth_url // use custom auth URL
        );

        // Initialize Ditto with persistent directory
        auto ditto = std::make_unique<Ditto>(identity, "./ditto_data");

        // Configure WebSocket transport
        auto config = ditto->get_transport_config();
        config.connect.websocket_urls.insert(websocket_url);
        config.enable_all_peer_to_peer();
        ditto->set_transport_config(config);

        // Enable DQL (disable v3 sync, disable strict mode)
        ditto->disable_sync_with_v3();

        // Start sync
        ditto->start_sync();
        std::cout << "Ditto sync started" << std::endl;

        // Create simulator with location
        PrinterSimulator simulator(printer_id, location);
        std::cout << "Simulator initialized" << std::endl;
        std::cout << "========================================" << std::endl;

        // Main simulation loop - 1Hz tick rate
        const auto tick_duration = milliseconds(1000);

        while (true) {
            auto tick_start = steady_clock::now();

            // Advance simulation
            simulator.tick();

            // Get telemetry and write to Ditto
            auto telemetry = simulator.getTelemetry();
            telemetry::writeTelemetry(ditto.get(), printer_id, telemetry);

            // Log status
            std::cout << "[" << printer_id << "] "
                      << simulator.getStatus();

            // Add job progress if printing
            if (simulator.getStatus() == "printing" && telemetry.contains("job")) {
                auto progress = telemetry["job"]["progress"].get<double>() * 100;
                std::cout << " - " << static_cast<int>(progress) << "% complete";
            }

            std::cout << std::endl;

            // Sleep until next tick (compensate for execution time)
            auto tick_end = steady_clock::now();
            auto elapsed = duration_cast<milliseconds>(tick_end - tick_start);
            if (elapsed < tick_duration) {
                std::this_thread::sleep_for(tick_duration - elapsed);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
