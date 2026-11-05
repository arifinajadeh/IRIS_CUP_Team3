
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

struct Telemetry
{
    int sequence;
    int rpm;
    float battery_voltage;
    std::string motor_status;
    int error_code;
};

// Memisahkan string berdasarkan koma
std::vector<std::string> split(
    const std::string& data,
    char delimiter
)
{
    std::vector<std::string> result;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, delimiter))
    {
        result.push_back(item);
    }

    return result;
}

// Mengubah telemetry mentah menjadi data terstruktur
bool parseTelemetry(
    const std::string& message,
    Telemetry& telemetry
)
{
    try
    {
        std::vector<std::string> parts = split(message, ',');

        // Memeriksa jumlah dan jenis paket
        if (parts.size() != 6 || parts[0] != "TEL")
        {
            return false;
        }

        telemetry.sequence = std::stoi(parts[1]);
        telemetry.rpm = std::stoi(parts[2]);
        telemetry.battery_voltage = std::stof(parts[3]);
        telemetry.motor_status = parts[4];
        telemetry.error_code = std::stoi(parts[5]);

        return true;
    }
    catch (...)
    {
        return false;
    }
}

int main()
{
    // Membuat UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        std::cerr << "Gagal membuat socket\n";
        return 1;
    }

    // Alamat Fake ESP32
    sockaddr_in esp_address{};

    esp_address.sin_family = AF_INET;
    esp_address.sin_port = htons(5005);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &esp_address.sin_addr
    );

    // Timeout penerimaan 500 ms
    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    );

    int sequence = 0;

    std::cout << "UDP Communication Started\n";

    while (true)
    {
        // Membuat command
        std::string command =
            "CMD," + std::to_string(sequence) +
            ",15.0,120,AUTO";

        // Mengirim command
        ssize_t sent = sendto(
            sock,
            command.c_str(),
            command.size(),
            0,
            (sockaddr*)&esp_address,
            sizeof(esp_address)
        );

        if (sent < 0)
        {
            std::cerr << "Gagal mengirim command\n";
            break;
        }

        std::cout << "\nCommand: " << command << '\n';

        // Menerima telemetry
        char buffer[1024];

        sockaddr_in sender_address{};
        socklen_t sender_length = sizeof(sender_address);

        ssize_t received = recvfrom(
            sock,
            buffer,
            sizeof(buffer) - 1,
            0,
            (sockaddr*)&sender_address,
            &sender_length
        );

        if (received >= 0)
        {
            buffer[received] = '\0';

            std::string message(buffer);

            Telemetry telemetry{};

            if (parseTelemetry(message, telemetry))
            {
                std::cout << "Telemetry berhasil diparsing:\n";

                std::cout << "Sequence : "
                          << telemetry.sequence << '\n';

                std::cout << "RPM      : "
                          << telemetry.rpm << '\n';

                std::cout << "Battery  : "
                          << telemetry.battery_voltage
                          << " V\n";

                std::cout << "Status   : "
                          << telemetry.motor_status << '\n';

                std::cout << "Error    : "
                          << telemetry.error_code << '\n';
            }
            else
            {
                std::cout << "Telemetry tidak valid: "
                          << message << '\n';
            }

        }
        else
        {
            std::cout << "Timeout: telemetry tidak diterima\n";
        }

        sequence++;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }

    close(sock);

    return 0;
}
