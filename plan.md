# Implementation Plan: ros_web_bridge Package

## Context
The user wants to create a ROS-Web bridge project with separate frontend and backend. The backend needs to:
1. Use rosbridge to expose ROS topics via WebSocket
2. Play a bag file when requested via a service
3. Publish `/camera/color/image_raw` topic from the bag file
4. Respond to frontend requests for start/stop playback

This is a learning project to understand the ROS → Web workflow with rosbridge_suite → WebSocket → roslibjs → React.

## Architecture Overview

```
ROS Backend (C++):
- ros_web_bridge_node: Main node with service for bag control
- BagPlayer: Helper class to manage rosbag playback
- rosbridge_server: Separate node for WebSocket bridge

Frontend (React/JavaScript):
- WebSocket connection to rosbridge (ws://localhost:9090)
- roslibjs for ROS communication
- Play button to trigger bag playback service
- Image display for camera stream
```

## Critical Files to Create/Modify

### Backend (ROS Package: `/home/robot/projects/catkin_ws2/src/ros_web_bridge/`)

1. **Service Definition** (`srv/PlayBag.srv`) - NEEDS UPDATE
   ```
   bool start   # true = start, false = stop
   ---
   bool success # true if operation succeeded
   ```

2. **Header File** (`include/ros_web_bridge/bag_player.hpp`)
   - BagPlayer class with Start(), Stop(), IsRunning() methods
   - Thread-safe bag playback control
   - Filtered publishing of only `/camera/color/image_raw`

3. **Implementation** (`src/bag_player.cpp`)
   - Uses rosbag::Player for bag playback
   - Runs in separate thread for non-blocking operation
   - Publishes filtered image messages

4. **Main Node** (`src/ros_web_bridge_node.cpp`)
   - ROS node with PlayBag service
   - Instantiates BagPlayer with hardcoded bag path
   - Service callback to start/stop playback

5. **Launch File** (`launch/ros_web_bridge.launch`)
   - Starts rosbridge_server node on port 9090
   - Starts ros_web_bridge_node
   - Allows WebSocket connections from frontend

6. **Build Configuration Updates** (`CMakeLists.txt`, `package.xml`)
   - Add rosbag dependency (already in package.xml)
   - Enable service generation
   - Add executable target

### Frontend (Outside catkin workspace, e.g., `/home/robot/projects/web_frontend/`)

1. **React Application** with:
   - WebSocket connection to rosbridge
   - roslibjs integration
   - Play/Stop buttons
   - Image display component

## Implementation Order

### Phase 1: Backend ROS Package
1. Update `PlayBag.srv` to simple bool format
2. Create `bag_player.hpp` header
3. Implement `bag_player.cpp` 
4. Create `ros_web_bridge_node.cpp`
5. Update `CMakeLists.txt` for service generation and dependencies
6. Create launch file
7. Build and test with `catkin_make`

### Phase 2: Frontend React Application  
1. Initialize React project at `/home/robot/projects/web_frontend/`
2. Install roslibjs dependencies
3. Create WebSocket connection component
4. Implement service call for bag control
5. Create image stream display
6. Test end-to-end connection

### Phase 3: Integration Testing
1. Launch backend with `roslaunch`
2. Connect frontend via WebSocket
3. Test play/stop functionality
4. Verify image streaming

## Key Design Decisions

1. **Single Backend Node**: One custom node manages bag playback and service, separate from rosbridge_server for cleaner separation.

2. **Threaded Bag Playback**: Bag player runs in separate thread to avoid blocking ROS spin, allows responsive service calls.

3. **Hardcoded Bag Path**: Initially hardcoded as per requirements, can be parameterized later.

4. **Filtered Publishing**: Only `/camera/color/image_raw` is republished to reduce network traffic.

5. **Standard rosbridge**: Use official rosbridge_server package rather than custom WebSocket implementation.

## Verification Steps

1. **Build Verification**: `catkin_make ros_web_bridge` compiles without errors
2. **Service Test**: `rosservice call /ros_web_bridge_node/play_bag` works
3. **Topic Verification**: `/camera/color/image_raw` publishes when bag plays
4. **WebSocket Test**: Connect to `ws://localhost:9090` from browser
5. **Frontend Test**: Play button triggers bag playback, images display

## Existing Patterns to Reuse

From `lidar_camera_projection` package:
- Google C++ style (snake_case, PascalCase)
- ROS parameter loading pattern
- Class-based node design
- RAII pattern for ROS resources

## Files to Reference
- `/home/robot/projects/catkin_ws2/src/lidar_camera_projection/src/lidar_filter_node.cpp` - Example ROS node pattern
- `/home/robot/projects/catkin_ws2/src/lidar_camera_projection/CMakeLists.txt` - Build configuration
- `/home/robot/projects/catkin_ws2/src/ros_web_bridge/srv/PlayBag.srv` - Existing service definition (to be updated)