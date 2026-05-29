#  ros_web_bridge Package

## 1 Overview
Bridge ROS data to web interfaces via WebSocket using rosbridge_suite. Provides bag file playback control and camera image streaming to web frontend.

**Package**: `ros_web_bridge` v0.1.0  
**Path**: `src/ros_web_bridge/`  
**Build**: Catkin (ROS1 Noetic)

## 2 Architecture

```
ROS Backend (C++):
┌─────────────────────┐       ┌─────────────────────┐
│ ros_web_bridge_node │ ◄──── │      BagPlayer      │
│   - PlayBag service │       │  - Threaded player  │
│   - WebSocket conn  │       │  - Filtered publish │
└─────────┬───────────┘       └─────────────────────┘
          │ WebSocket (9090)
          ▼
┌─────────────────────┐
│ rosbridge_server    │
│   - WebSocket bridge│
└─────────┬───────────┘
          │ WebSocket
          ▼
┌─────────────────────┐
│ Frontend (React)    │
│   - roslibjs client │
│   - UI controls     │
│   - Image display   │
└─────────────────────┘
```

**Bag File Flow**: `lvdata_2026-05-11-15-54-05.bag` → `BagPlayer` → `/camera/color/image_raw` topic → through ROS topics → via rosbridge → to web frontend

## 3 ROS Topic and Service Map

| Topic/Service | Type | Publisher | Subscriber | Direction |
|--------------|------|-----------|------------|-----------|
| `/camera/color/image_raw` | `sensor_msgs/Image` | `BagPlayer` (via bag) | Web frontend via rosbridge | Out |
| `/camera/color/camera_info` | `sensor_msgs/CameraInfo` | `BagPlayer` (via bag) | Web frontend via rosbridge | Out |
| `/ros_web_bridge_node/play_bag` | `ros_web_bridge/PlayBag` | React frontend | `ros_web_bridge_node` | In |

## 4 Module Details

### 4.1 BagPlayer
- **Source**: `src/bag_player.cpp`, `include/ros_web_bridge/bag_player.hpp`
- **Class**: `BagPlayer`
- **Features**: Thread-safe bag playback wrapper using `rosbag::Bag` and `rosbag::View`
- **Filtering**: Publishes `/camera/color/image_raw` and `/camera/color/camera_info` topics from bag file
- **Threading**: Runs in separate thread to avoid blocking ROS service calls

### 4.2 ros_web_bridge_node
- **Source**: `src/ros_web_bridge_node.cpp`
- **Class**: Main ROS node
- **Service**: `/ros_web_bridge_node/play_bag` (bool start → bool success)
- **Integration**: Works with `rosbridge_server` for WebSocket communication

## 5 Web Frontend Interface
- **Location**: `/home/robot/projects/web_frontend/` (external to catkin workspace)
- **Technology**: React + TypeScript + Vite + roslibjs
- **Features**: WebSocket connection, play/stop controls, camera image streaming, camera info display (resolution, distortion model)
- **Connectivity**: WebSocket at `ws://localhost:9090`

## 6 External File Interfaces

| File | Reader | Format | Description |
|------|--------|--------|-------------|
| `lvdata_2026-05-11-15-54-05.bag` | `BagPlayer` | ROS bag | Camera image stream and calibration data |
| **Topics in bag**: `/camera/color/image_raw`, `/camera/color/camera_info`, `/livox/lidar` |

## 7 Known Limitations

1. No distortion correction (Brown-Conrady coeffs in calib.json ignored)
2. Two hardcoded topics: `/livox/filtered_pixel`, `/livox/inbox_voxel`
3. Hand-written JSON parser — only supports specific flat formats
4. `tf2_eigen` declared but unused in CMakeLists.txt
5. Detection loading via file polling (not ROS topic subscription)
6. `transformCloudToPixel` does not filter invalid points (N in = N out)

## 8 TODOs(User)
- [x] add "/camera/color/camera_info" topic - Added publisher for CameraInfo messages, BagPlayer now publishes both image_raw and camera_info topics
- [x] web show the picture's width and height - Frontend now subscribes to camera_info, displays resolution (width × height), distortion model, and update timestamp
