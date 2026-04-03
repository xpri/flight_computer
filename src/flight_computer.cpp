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
                {
                    // Place all code from telementary_callback here.
                    double current_altitude = msg->pose.position.z;

                    // Detect apogee when altitude starts decreasing
                    // If statement logic explained: if the parachute is deployed AND if the last altitude is greater than current altitude
                    // AND if last altitude is greater than one (as a fail safe) then the if statement executes.
                    if(!parachute_deployed && (last_altitude > current_altitude) && (last_altitude > 1.0))
                    {
                        RCLCPP_INFO(this->get_logger(), "APOGEE DETECTED at %.2f meters! Deploying parachute...", last_altitude);

                        auto cmd = std_msgs::msg::Bool();
                        cmd.data = true;
                        parachute_publish_->publish(cmd);

                        parachute_deployed = true;
                    }

                    last_altitude = current_altitude;
                }
            );

            //Initializes a topic called cmd parachute in boolean with a buffer of 10
            parachute_publish_ = this->create_publisher<std_msgs::msg::Bool>("cmd_parachute", 10);
            last_altitude = 0.0;
            parachute_deployed = false;
        }
    
    private:

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