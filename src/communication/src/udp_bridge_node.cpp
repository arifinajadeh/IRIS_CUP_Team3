
#include <memory>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"

class UDPBridgeNode : public rclcpp::Node
{
public:
    UDPBridgeNode() : Node("udp_bridge_node")
    {
        // Membuat UDP socket
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);

        if (sock_ < 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Gagal membuat UDP socket"
            );
            throw std::runtime_error("Socket creation failed");
        }

        // Konfigurasi alamat Fake ESP32
        esp_address_.sin_family = AF_INET;
        esp_address_.sin_port = htons(5005);

        inet_pton(
            AF_INET,
            "127.0.0.1",
            &esp_address_.sin_addr
        );

        // Subscriber command ROS 2
        command_subscriber_ =
            this->create_subscription<
                std_msgs::msg::Float32MultiArray
            >(
                "/robot/command",
                10,
                std::bind(
                    &UDPBridgeNode::commandCallback,
                    this,
                    std::placeholders::_1
                )
            );

        RCLCPP_INFO(
            this->get_logger(),
            "UDP Bridge Node berhasil dijalankan"
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Menunggu command dari /robot/command"
        );
    }

    ~UDPBridgeNode()
    {
        if (sock_ >= 0)
        {
            close(sock_);
        }
    }

private:

    void commandCallback(
        const std_msgs::msg::Float32MultiArray::SharedPtr msg
    )
    {
        // Memeriksa jumlah data
        if (msg->data.size() != 2)
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Command harus berisi steering dan speed"
            );
            return;
        }

        float steering = msg->data[0];
        float speed = msg->data[1];

        // Membuat command UDP
        std::string command =
            "CMD," + std::to_string(sequence_) +
            "," + std::to_string(steering) +
            "," + std::to_string(speed) +
            ",AUTO";

        // Mengirim command ke Fake ESP32
        ssize_t sent = sendto(
            sock_,
            command.c_str(),
            command.size(),
            0,
            (sockaddr*)&esp_address_,
            sizeof(esp_address_)
        );

        if (sent < 0)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Gagal mengirim UDP"
            );
            return;
        }

        RCLCPP_INFO(
            this->get_logger(),
            "UDP dikirim: %s",
            command.c_str()
        );

        sequence_++;
    }

    int sock_ = -1;
    int sequence_ = 0;

    sockaddr_in esp_address_{};

    rclcpp::Subscription<
        std_msgs::msg::Float32MultiArray
    >::SharedPtr command_subscriber_;
};


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<UDPBridgeNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
