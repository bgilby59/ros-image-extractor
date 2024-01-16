#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/compressed_image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv4/opencv2/opencv.hpp"

class ImageExtractor : public rclcpp::Node
{
public:
    ImageExtractor() : Node("image_extractor"), image_counter_(0)
    {
        this->declare_parameter("image_topic", "/image_raw/compressed");
        image_topic_ = this->get_parameter("image_topic").as_string();

        image_subscriber_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(image_topic_, 50, std::bind(&ImageExtractor::image_topic_callback, this, std::placeholders::_1));
    }

private:
    void image_topic_callback(const sensor_msgs::msg::CompressedImage image_msg)
    {
        auto format_encoding_pair = get_format_and_encoding(image_msg.format);
        auto format = format_encoding_pair.first;
        auto encoding = format_encoding_pair.second;

        cv_bridge::CvImagePtr cv_image = cv_bridge::toCvCopy(image_msg, encoding);

        std::cout << "writing to image file "
                  << "calibration_image" + std::to_string(image_counter_) + "." + format << std::endl
                  << std::flush;

        if (cv::imwrite("../calibration_images/calibration_image" + std::to_string(image_counter_++) + "." + format, cv_image->image))
            std::cout << "Successfully wrote image to file!" << std::endl
                      << std::flush;

        return;
    }

    std::pair<std::string, std::string> get_format_and_encoding(std::string format)
    {
        int format_start = format.find(" ", 0);
        int format_end = format.find(" ", format_start + 1);

        return std::pair(format.substr(format_start + 1, format_end - format_start), format.substr(format.length() - 4, 4));
    }

    std::string image_topic_;
    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr image_subscriber_;
    int image_counter_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ImageExtractor>());
    rclcpp::shutdown();
    return 0;
}