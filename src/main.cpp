#include "LIVMapper.h"

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("laserMapping");
  LIVMapper mapper(node);
  mapper.initializeSubscribersAndPublishers();
  mapper.run();
  rclcpp::shutdown();
  return 0;
}
