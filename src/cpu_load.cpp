#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class CPULoadPublisher : public rclcpp::Node
{
public:
  CPULoadPublisher() : Node("cpu_load_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>("cpu_load", 10);
    timer_ = this->create_wall_timer(std::chrono::seconds(5), std::bind(&CPULoadPublisher::publishCPULoad, this));
    logfile_.open("cpu_load.log", std::ios::out | std::ios::app);
  }

private:
  void publishCPULoad() // take a look in this function, there is at least one thing to change
  {
    //create an offset
    //create a multiplier
    std_msgs::msg::String msg;
    msg.data = std::to_string(getCurrentCPULoad()); //add the offset * multiplier
    publisher_->publish(msg); //send the relative CPU load
    logfile_ << msg.data << std::endl;
    ++count_;
  }

  float getCurrentCPULoad() // take a look in this function, there is at least one thing to change
  {
    // Get the current CPU load using the process ID (PID)
    pid_t pid = getpid(); 
    // try to get all PID from the CPU
    std::string command = "ps -p " + std::to_string(pid) + " -o %cpu --no-headers";
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
      RCLCPP_ERROR(this->get_logger(), "Failed to run command to get CPU load");
      return 0.0;
    }

    float cpu_load = 0.0;
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      cpu_load = std::strtof(buffer, nullptr);
    }

    pclose(pipe);
    //test without /100 to see what happens
    return cpu_load / 100.0; // Normalize the CPU load to a relative value between 0.0 and 1.0
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_;
  std::ofstream logfile_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CPULoadPublisher>());
  rclcpp::shutdown();
  return 0;
}
