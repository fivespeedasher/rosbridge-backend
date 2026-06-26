#include "ros_web_bridge/bag_player.hpp"

#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/PointCloud2.h>
#include <ros/console.h>

namespace ros_web_bridge {

BagPlayer::BagPlayer(const ros::NodeHandle& nh, const std::string& bag_path)
    : nh_(nh), bag_path_(bag_path) {
  // Create publisher for camera images
  image_pub_ = nh_.advertise<sensor_msgs::Image>("/camera/color/image_raw", 10);
  // Create publisher for camera info
  camera_info_pub_ = nh_.advertise<sensor_msgs::CameraInfo>("/camera/color/camera_info", 10);
  // Create publisher for LiDAR point cloud
  lidar_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("/livox/lidar", 10);
}

BagPlayer::~BagPlayer() {
  // Ensure playback is stopped on destruction
  Stop();
}

bool BagPlayer::Start() {
  if (is_running_) {
    ROS_WARN("[BagPlayer] Playback already running");
    return false;
  }

  // Start playback in background thread
  is_running_ = true;
  playback_thread_ = std::thread(&BagPlayer::PlaybackThread, this);

  ROS_INFO("[BagPlayer] Started playback from bag: %s", bag_path_.c_str());
  ROS_INFO("[BagPlayer] Playback will loop continuously until stopped");
  return true;
}

bool BagPlayer::Stop() {
  if (!is_running_) {
    ROS_WARN("[BagPlayer] Playback not running");
    return false;
  }

  // Signal stop to thread
  ROS_INFO("[BagPlayer] Stopping playback...");
  is_running_ = false;

  // Wait for thread to finish
  if (playback_thread_.joinable()) {
    playback_thread_.join();
  }

  ROS_INFO("[BagPlayer] Playback stopped successfully");
  return true;
}

void BagPlayer::PlaybackThread() {
  try {
    int loop_count = 0;

    // Continuous playback loop
    while (is_running_) {
      loop_count++;
      ROS_INFO("[BagPlayer] Starting playback loop #%d", loop_count);

      // Open bag file
      rosbag::Bag bag;
      bag.open(bag_path_, rosbag::bagmode::Read);

      // Create view for the bag
      rosbag::View view(bag);

      // Get start time of the bag
      ros::Time bag_start_time = view.getBeginTime();
      ros::Time bag_end_time = view.getEndTime();

      ROS_INFO("[BagPlayer] Bag duration: %.1f seconds",
               (bag_end_time - bag_start_time).toSec());

      // Wall-clock start for rate-controlled playback
      ros::Time playback_start = ros::Time::now();

      // Playback loop
      for (const rosbag::MessageInstance& m : view) {
        // Check if we should stop
        if (!is_running_) {
          break;
        }

        // Forward camera image messages
        if (m.getTopic() == "/camera/color/image_raw") {
          sensor_msgs::Image::ConstPtr image_msg = m.instantiate<sensor_msgs::Image>();
          if (image_msg) {
            // Publish with original timestamp
            image_pub_.publish(image_msg);
          }
        }
        // Forward camera info messages
        else if (m.getTopic() == "/camera/color/camera_info") {
          sensor_msgs::CameraInfo::ConstPtr camera_info_msg = m.instantiate<sensor_msgs::CameraInfo>();
          if (camera_info_msg) {
            // Publish with original timestamp
            camera_info_pub_.publish(camera_info_msg);
          }
        }
        // Forward LiDAR point cloud messages
        else if (m.getTopic() == "/livox/lidar") {
          sensor_msgs::PointCloud2::ConstPtr lidar_msg = m.instantiate<sensor_msgs::PointCloud2>();
          if (lidar_msg) {
            // Publish with original timestamp
            lidar_pub_.publish(lidar_msg);
          }
        }

        // Honor original message timing: sleep until the next message's
        // wall-clock time matches its offset from the bag start.
        ros::Duration elapsed = m.getTime() - bag_start_time;
        ros::Time target_time = playback_start + elapsed;
        ros::Time now = ros::Time::now();
        if (target_time > now) {
          (target_time - now).sleep();
        }
      }

      // Close bag
      bag.close();

      if (is_running_) {
        ROS_INFO("[BagPlayer] Completed playback loop #%d", loop_count);
      }
    }

  } catch (const rosbag::BagException& e) {
    ROS_ERROR("[BagPlayer] Failed to open bag file '%s': %s",
              bag_path_.c_str(), e.what());
  } catch (const std::exception& e) {
    ROS_ERROR("[BagPlayer] Playback thread error: %s", e.what());
  }

  // Thread finished, mark as not running
  is_running_ = false;
  ROS_INFO("[BagPlayer] Playback thread finished");
}

}  // namespace ros_web_bridge