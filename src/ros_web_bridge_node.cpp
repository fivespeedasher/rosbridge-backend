#include <ros/ros.h>
#include <ros_web_bridge/PlayBag.h>
#include "ros_web_bridge/bag_player.hpp"

class RosWebBridgeNode {
 public:
  RosWebBridgeNode()
      : nh_private_("~") {
    // Get bag path from parameter, default to the known location
    nh_private_.param<std::string>("bag_path", bag_path_,
                                   "/home/robot/data/calib/lidar_vision_ip2/lvdata_2026-05-11-15-54-05.bag");

    // Initialize bag player with the path
    bag_player_.reset(new ros_web_bridge::BagPlayer(nh_private_, bag_path_));

    // Advertise the PlayBag service
    srv_ = nh_private_.advertiseService("play_bag",
                                        &RosWebBridgeNode::SrvCallback,
                                        this);

    ROS_INFO("[RosWebBridge] Node initialized");
    ROS_INFO("[RosWebBridge] Bag path: %s", bag_path_.c_str());
    ROS_INFO("[RosWebBridge] Service available: %s/play_bag",
             nh_private_.getNamespace().c_str());
  }

 private:
  /**
   * @brief Service callback for starting/stopping bag playback
   * @param req Service request (start = true to start, false to stop)
   * @param resp Service response (success = operation result)
   * @return true if service call was handled
   */
  bool SrvCallback(ros_web_bridge::PlayBag::Request& req,
                   ros_web_bridge::PlayBag::Response& resp) {
    if (req.start) {
      resp.success = bag_player_->Start();
      ROS_INFO("[RosWebBridge] Service: start playback -> %s",
               resp.success ? "success" : "failed");
    } else {
      resp.success = bag_player_->Stop();
      ROS_INFO("[RosWebBridge] Service: stop playback -> %s",
               resp.success ? "success" : "failed");
    }
    return true;
  }

  ros::NodeHandle nh_private_;
  std::string bag_path_;
  std::unique_ptr<ros_web_bridge::BagPlayer> bag_player_;
  ros::ServiceServer srv_;
};

int main(int argc, char** argv) {
  // Initialize ROS node
  ros::init(argc, argv, "ros_web_bridge_node");

  // Create node instance
  RosWebBridgeNode node;

  // Spin to handle service calls
  ros::spin();

  return 0;
}