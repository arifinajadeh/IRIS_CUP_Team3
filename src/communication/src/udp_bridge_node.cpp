
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class UDPBridgeNode : public rclcpp::Node
{
public:
    UDPBridgeNode() : Node("udp_bridge_node")
    {
        RCLCPP_INFO(
            this->get_logger(),
            "UDP Bridge Node berhasil dijalankan!"
        );
    }
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<UDPBridgeNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
    