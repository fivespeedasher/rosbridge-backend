#ifndef ROS_WEB_BRIDGE_BAG_PLAYER_HPP_
#define ROS_WEB_BRIDGE_BAG_PLAYER_HPP_

#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/PointCloud2.h>
#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace ros_web_bridge {

class BagPlayer {
 public:
  /**
   * @brief Constructor
   * @param nh Node handle for creating publishers
   * @param bag_path Full path to the bag file
   */
  explicit BagPlayer(const ros::NodeHandle& nh, const std::string& bag_path);

  /**
   * @brief Destructor
   */
  ~BagPlayer();

  /**
   * @brief Start bag playback in a background thread
   * @return true if playback started successfully, false otherwise
   */
  bool Start();

  /**
   * @brief Stop bag playback and join the background thread
   * @return true if playback stopped successfully, false otherwise
   */
  bool Stop();

  /**
   * @brief Check if playback is currently active
   * @return true if playback is running, false otherwise
   */
  bool IsRunning() const { return is_running_; }

 private:
  /**
   * @brief Main playback thread function
   */
  void PlaybackThread();

  ros::NodeHandle nh_;
  std::string bag_path_;
  ros::Publisher image_pub_;
  ros::Publisher camera_info_pub_;
  ros::Publisher lidar_pub_;
  std::thread playback_thread_;
  std::atomic<bool> is_running_{false};
};

}  // namespace ros_web_bridge

#endif  // ROS_WEB_BRIDGE_BAG_PLAYER_HPP_