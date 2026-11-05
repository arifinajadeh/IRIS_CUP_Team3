
#include <memory>
#include <string>
#include <chrono>
#include <functional>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <std_msgs/msg/header.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class CameraNode : public rclcpp::Node
{
public:
    CameraNode() : Node("camera_node")
    {
        this->declare_parameter<std::string>(
            "camera_url",
            "http://admin:121010@10.163.112.58:8081/video"
        );

        std::string camera_url =
            this->get_parameter("camera_url").as_string();

        publisher_ = this->create_publisher<sensor_msgs::msg::Image>(
            "/camera/image_raw",10
        );

        cap_.open(camera_url);

        if (!cap_.isOpened()) {
            RCLCPP_ERROR(
                this->get_logger(),
                "Gagal membuka kamera HP!"
            );
            return;
        }

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(33),
            std::bind(&CameraNode::publishFrame, this)
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Kamera berhasil terhubung!"
        );
    }

private:
    void publishFrame()
    {
        cv::Mat frame;

        if (!cap_.read(frame) || frame.empty()) {
            RCLCPP_WARN(
                this->get_logger(),
                "Gagal membaca frame kamera"
            );
            return;
        }

        std_msgs::msg::Header header;
        header.stamp = this->now();
        header.frame_id = "camera_frame";

        auto msg = cv_bridge::CvImage(
            header,
            sensor_msgs::image_encodings::BGR8,
            frame
        ).toImageMsg();

        publisher_->publish(*msg);
    }

    cv::VideoCapture cap_;

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;

    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<CameraNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
