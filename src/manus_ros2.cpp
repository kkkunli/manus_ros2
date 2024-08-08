/// @file manus_ros2.cpp
/// @brief This file contains the main function for the manus_ros2 node, which interfaces with the Manus SDK to
/// receive animated skeleton data, and republishes the events as ROS 2 messages.

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <sys/time.h>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "geometry_msgs/msg/pose_array.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"
#include "SDKMinimalClient.hpp"
#include <fstream>
#include <iostream>
#include <thread>


using namespace std::chrono_literals;
using namespace std;


/// @brief ROS2 publisher class for the manus_ros2 node
class ManusROS2Publisher : public rclcpp::Node
{
public:
	ManusROS2Publisher() : Node("manus_ros2")
	{
		manus_left_publisher_ = this->create_publisher<geometry_msgs::msg::PoseArray>("manus_left", 10);
    	manus_right_publisher_ = this->create_publisher<geometry_msgs::msg::PoseArray>("manus_right", 10);
		manus_ergonomics_publisher = this->create_publisher<sensor_msgs::msg::JointState>("manus_ergonomics", 10);
		manus_leftTrackerData_publisher_ = this->create_publisher<geometry_msgs::msg::Pose>("manus_tracker_left", 10);
		manus_rightTrackerData_publisher_ = this->create_publisher<geometry_msgs::msg::Pose>("manus_tracker_right", 10);
	}

	void publish_left(geometry_msgs::msg::PoseArray::SharedPtr pose_array) {
    	manus_left_publisher_->publish(*pose_array);
  	}

  	void publish_right(geometry_msgs::msg::PoseArray::SharedPtr pose_array) {
    	manus_right_publisher_->publish(*pose_array);
  	}

	void publish_ergonomics(sensor_msgs::msg::JointState::SharedPtr ergonomics_data) {
		manus_ergonomics_publisher->publish(*ergonomics_data);
	}

	void publish_leftTrackerData(geometry_msgs::msg::Pose::SharedPtr pose) {
    	manus_leftTrackerData_publisher_->publish(*pose);
  	}

	void publish_rightTrackerData(geometry_msgs::msg::Pose::SharedPtr pose) {
    	manus_rightTrackerData_publisher_->publish(*pose);
  	}

private:
	rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr manus_left_publisher_;
  	rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr manus_right_publisher_;
	rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr manus_ergonomics_publisher;
	rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr manus_leftTrackerData_publisher_;
  	rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr manus_rightTrackerData_publisher_;
};



void convertSkeletonDataToROS(std::shared_ptr<ManusROS2Publisher> publisher)
{
	ClientSkeletonCollection* csc = SDKMinimalClient::GetInstance()->CurrentSkeletons();
	if (csc != nullptr && csc->skeletons.size() != 0) {
    	for (size_t i=0; i < csc->skeletons.size(); ++i) {

			// Prepare a new PoseArray message for the data
			auto pose_array = std::make_shared<geometry_msgs::msg::PoseArray>();
			pose_array->header.stamp = publisher->now();

			// Set the poses for the message
      		for (size_t j=0; j < csc->skeletons[i].info.nodesCount; ++j) {
        		const auto &joint = csc->skeletons[i].nodes[j];
				geometry_msgs::msg::Pose pose;
				pose.position.x = joint.transform.position.x;
				pose.position.y = joint.transform.position.y;
				pose.position.z = joint.transform.position.z;
				pose.orientation.x = joint.transform.rotation.x;
				pose.orientation.y = joint.transform.rotation.y;
				pose.orientation.z = joint.transform.rotation.z;
				pose.orientation.w = joint.transform.rotation.w;
				pose_array->poses.push_back(pose);
			}

			// Which hand is this?
			if (csc->skeletons[i].info.id == SDKMinimalClient::GetInstance()->GetRightHandID()) {
				pose_array->header.frame_id = "manus_right";
				publisher->publish_right(pose_array);
			} else {
				pose_array->header.frame_id = "manus_left";
				publisher->publish_left(pose_array);
			}
		}
	}
}

void convertErgonomicsDataToROS(std::shared_ptr<ManusROS2Publisher> publisher)
{
	ClientErgonomics* ce = SDKMinimalClient::GetInstance()->CurrentErgonomics();
	if (ce != nullptr) {

		// Prepare a JointState message for the data
		auto ergonomics_data = std::make_shared<sensor_msgs::msg::JointState>();
		ergonomics_data->header.stamp = publisher->now();

		// Set the data for the message
		ergonomics_data->name = {
			"LeftFingerThumbMCPSpread",
			"LeftFingerThumbMCPStretch",
			"LeftFingerThumbPIPStretch",
			"LeftFingerThumbDIPStretch",

			"LeftFingerIndexMCPSpread",
			"LeftFingerIndexMCPStretch",
			"LeftFingerIndexPIPStretch",
			"LeftFingerIndexDIPStretch",

			"LeftFingerMiddleMCPSpread",
			"LeftFingerMiddleMCPStretch",
			"LeftFingerMiddlePIPStretch",
			"LeftFingerMiddleDIPStretch",

			"LeftFingerRingMCPSpread",
			"LeftFingerRingMCPStretch",
			"LeftFingerRingPIPStretch",
			"LeftFingerRingDIPStretch",

			"LeftFingerPinkyMCPSpread",
			"LeftFingerPinkyMCPStretch",
			"LeftFingerPinkyPIPStretch",
			"LeftFingerPinkyDIPStretch",

			"RightFingerThumbMCPSpread",
			"RightFingerThumbMCPStretch",
			"RightFingerThumbPIPStretch",
			"RightFingerThumbDIPStretch",

			"RightFingerIndexMCPSpread",
			"RightFingerIndexMCPStretch",
			"RightFingerIndexPIPStretch",
			"RightFingerIndexDIPStretch",

			"RightFingerMiddleMCPSpread",
			"RightFingerMiddleMCPStretch",
			"RightFingerMiddlePIPStretch",
			"RightFingerMiddleDIPStretch",

			"RightFingerRingMCPSpread",
			"RightFingerRingMCPStretch",
			"RightFingerRingPIPStretch",
			"RightFingerRingDIPStretch",

			"RightFingerPinkyMCPSpread",
			"RightFingerPinkyMCPStretch",
			"RightFingerPinkyPIPStretch",
			"RightFingerPinkyDIPStretch"
		};   // Mirrors the definition of typedef enum ErgonomicsDataType

		// Reserve space for all positions based on the maximum size of the enum
		ergonomics_data->position.reserve(ErgonomicsDataType_MAX_SIZE);
		for (int i = 0; i < ErgonomicsDataType_MAX_SIZE; i++) {
			ergonomics_data->position.push_back(0.0);
		}

		// Set positions for each joint from the data source for the left hand
		ergonomics_data->position[ErgonomicsDataType_LeftFingerThumbMCPSpread] = ce->data_left->data[ErgonomicsDataType_LeftFingerThumbMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerThumbMCPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerThumbMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerThumbPIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerThumbPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerThumbDIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerThumbDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_LeftFingerIndexMCPSpread] = ce->data_left->data[ErgonomicsDataType_LeftFingerIndexMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerIndexMCPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerIndexMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerIndexPIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerIndexPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerIndexDIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerIndexDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_LeftFingerMiddleMCPSpread] = ce->data_left->data[ErgonomicsDataType_LeftFingerMiddleMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerMiddleMCPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerMiddleMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerMiddlePIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerMiddlePIPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerMiddleDIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerMiddleDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_LeftFingerRingMCPSpread] = ce->data_left->data[ErgonomicsDataType_LeftFingerRingMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerRingMCPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerRingMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerRingPIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerRingPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerRingDIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerRingDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_LeftFingerPinkyMCPSpread] = ce->data_left->data[ErgonomicsDataType_LeftFingerPinkyMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerPinkyMCPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerPinkyMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerPinkyPIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerPinkyPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_LeftFingerPinkyDIPStretch] = ce->data_left->data[ErgonomicsDataType_LeftFingerPinkyDIPStretch];

		// Set positions for each joint from the data source for the right hand
		ergonomics_data->position[ErgonomicsDataType_RightFingerThumbMCPSpread] = ce->data_right->data[ErgonomicsDataType_RightFingerThumbMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_RightFingerThumbMCPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerThumbMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerThumbPIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerThumbPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerThumbDIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerThumbDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_RightFingerIndexMCPSpread] = ce->data_right->data[ErgonomicsDataType_RightFingerIndexMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_RightFingerIndexMCPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerIndexMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerIndexPIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerIndexPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerIndexDIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerIndexDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_RightFingerMiddleMCPSpread] = ce->data_right->data[ErgonomicsDataType_RightFingerMiddleMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_RightFingerMiddleMCPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerMiddleMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerMiddlePIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerMiddlePIPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerMiddleDIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerMiddleDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_RightFingerRingMCPSpread] = ce->data_right->data[ErgonomicsDataType_RightFingerRingMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_RightFingerRingMCPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerRingMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerRingPIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerRingPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerRingDIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerRingDIPStretch];

		ergonomics_data->position[ErgonomicsDataType_RightFingerPinkyMCPSpread] = ce->data_right->data[ErgonomicsDataType_RightFingerPinkyMCPSpread];
		ergonomics_data->position[ErgonomicsDataType_RightFingerPinkyMCPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerPinkyMCPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerPinkyPIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerPinkyPIPStretch];
		ergonomics_data->position[ErgonomicsDataType_RightFingerPinkyDIPStretch] = ce->data_right->data[ErgonomicsDataType_RightFingerPinkyDIPStretch];


		// Publish the message
		publisher->publish_ergonomics(ergonomics_data);
	}
}


void convertTrackerDataToROS(std::shared_ptr<ManusROS2Publisher> publisher)
{
	TrackerDataCollection* tdc = SDKMinimalClient::GetInstance()->CurrentTrackerData();
	if (tdc != nullptr && tdc->trackerData.size() != 0){
    	for (size_t i=0; i < tdc->trackerData.size(); ++i) {
			// Prepare a new Pose message for the data
            auto pose = std::make_shared<geometry_msgs::msg::Pose>();

			// Set the poses for the message
			const auto &data = tdc->trackerData[i];

			pose->position.x = data.position.x;
			pose->position.y = data.position.y;
			pose->position.z = data.position.z;
			pose->orientation.x = data.rotation.x;
			pose->orientation.y = data.rotation.y;
			pose->orientation.z = data.rotation.z;
			pose->orientation.w = data.rotation.w;

			// Which hand is this?
			if (tdc->trackerData[i].trackerType == TrackerType_RightHand){
				publisher->publish_rightTrackerData(pose);
			}
			else if (tdc->trackerData[i].trackerType == TrackerType_LeftHand){
				publisher->publish_leftTrackerData(pose);
			}
		}
	}
}

// Main function - Initializes the minimal client and starts the ROS2 node
int main(int argc, char *argv[]) 
{
	rclcpp::init(argc, argv);

	auto publisher = std::make_shared<ManusROS2Publisher>();

	RCLCPP_INFO(publisher->get_logger(), "Starting manus_ros2 node");
	SDKMinimalClient t_Client(publisher);
	ClientReturnCode status = t_Client.Initialize();

	if (status != ClientReturnCode::ClientReturnCode_Success)
	{
		RCLCPP_ERROR_STREAM(publisher->get_logger(), "Failed to initialize the Manus SDK. Error code: " << (int)status);
		return 1;
	}

	RCLCPP_INFO(publisher->get_logger(), "Connecting to Manus SDK");
	t_Client.ConnectToHost();


	// Create an executor to spin the minimal_publisher
	auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
	executor->add_node(publisher);

	// Publish the poses at 50hz
	auto timer = publisher->create_wall_timer(
		20ms,
		[&t_Client,&publisher]() {
			if (t_Client.Run()) {
				convertSkeletonDataToROS(publisher);
				convertErgonomicsDataToROS(publisher);
				convertTrackerDataToROS(publisher);
			}
		}
	);

	// Spin the executor
	executor->spin();

	// Shutdown the Manus client
	t_Client.ShutDown();

	// Shutdown ROS 2
	rclcpp::shutdown();

	return 0;
}
