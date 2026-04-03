#include <chrono>
#include <functional>
#include <memory>
#include <string>

// Include the ROS 2 C++ client library
#include "rclcpp/rclcpp.hpp"
// Include the standard string message type library
#include "std_msgs/msg/string.hpp"
// For the ability to use boolean arithmetic
#include "std_msgs/msg/bool.hpp"

#include "geometry_msgs/msg/pose_stamped.hpp"

// To use s, ms, etc
using namespace std::chrono_literals;

class FlightComputerNode : public rclcpp::Node
{
    public:
        FlightComputerNode()
            : Node("flight_computer"),
            count_(0)
        {
            subscription_ = this->create_subscription<geometry_msgs::msg::PoseStamped>
            (
                "rocket_pose",          // Subscribes to topic called "rocket_pose"
                10,
                
                [this](const geometry_msgs::msg::PoseStamped::SharedPtr msg)           // Lambda function for readability
                {}
            );
            parachute_publish_ = this->create_publisher<std_msgs::msg::Bool>("cmd_parachute", 10);      // Initializes a topic called cmd parachute in boolean with a buffer of 10
            
            last_altitude = 0.0;
            parachute_deployed = false;
        }
    
    private:
        void telementary_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
        {
            double current_altitude = msg->pose.position.z;

            // Detect apogee when altitude starts decreasing
            if((last_altitude > current_altitude) || (parachute_deployed == true))      // If the last altitude is greater than the current then the peak was reached.
            {
                RCLCPP_INFO(this->get_logger(), "APOGEE DETECTED at %.2f meters! Deploying parachute...", last_altitude);

                auto cmd = std_msgs::msg::Bool();
                cmd.data = true;
                parachute_publish_->publish(cmd);

                parachute_deployed = true;
            }

            last_altitude = current_altitude;

        }
        // Must include SharedPtr in node decleration
        rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription_;
        rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr parachute_publish_;
        
        // Declare all variables used.
        double last_altitude;
        bool parachute_deployed;
        size_t count_;

};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<FlightComputerNode>());
    rclcpp::shutdown();
    return 0;
}