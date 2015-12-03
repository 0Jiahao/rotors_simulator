/*
 * Copyright 2015 Fadri Furrer, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Michael Burri, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Mina Kamel, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Janosch Nikolic, ASL, ETH Zurich, Switzerland
 * Copyright 2015 Markus Achtelik, ASL, ETH Zurich, Switzerland
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include <chrono>

#include <Eigen/Core>
#include <mav_msgs/conversions.h>
#include <mav_msgs/default_topics.h>
#include <manipulator_msgs/default_topics_manipulator.h>
#include <manipulator_msgs/conversions.h>
#include <ros/ros.h>
#include <std_srvs/Empty.h>
#include <trajectory_msgs/MultiDOFJointTrajectory.h>
#include <trajectory_msgs/JointTrajectory.h>


int main(int argc, char** argv){
  ros::init(argc, argv, "hovering_example_qp");

  ros::NodeHandle nh;
  ros::Publisher trajectory_pub = nh.advertise<trajectory_msgs::MultiDOFJointTrajectory>(mav_msgs::default_topics::COMMAND_TRAJECTORY, 10);
  ros::Publisher joint_trajectory_pub = nh.advertise<trajectory_msgs::JointTrajectory>(manipulator_msgs::default_topics::COMMAND_JOINT_TRAJECTORY, 10);

  ROS_INFO("Started hovering example for aerial manipulator.");

  std_srvs::Empty srv;
  bool unpaused = ros::service::call("/gazebo/unpause_physics", srv);
  unsigned int i = 0;

  // Trying to unpause Gazebo for 10 seconds.
  while (i <= 10 && !unpaused) {
    ROS_INFO("Wait for 1 second before trying to unpause Gazebo again.");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    unpaused = ros::service::call("/gazebo/unpause_physics", srv);
    ++i;
  }

  if (!unpaused) {
    ROS_FATAL("Could not wake up Gazebo.");
    return -1;
  }
  else {
    ROS_INFO("Unpaused the Gazebo simulation.");
  }

  // Wait for 3 seconds to let the Gazebo GUI show up.
  ros::Duration(3.0).sleep();

  // Publish waypoint for helicopter
  trajectory_msgs::MultiDOFJointTrajectory trajectory_msg;
  trajectory_msg.header.stamp = ros::Time::now();
  Eigen::Vector3d desired_position(0.0, 0.0, 1.0);
  double desired_yaw = 0.0;
  mav_msgs::msgMultiDofJointTrajectoryFromPositionYaw(desired_position,
      desired_yaw, &trajectory_msg);

  ROS_INFO("Publishing helicopter waypoint on namespace %s: [%f, %f, %f].",
           nh.getNamespace().c_str(),
           desired_position.x(),
           desired_position.y(),
           desired_position.z());
  trajectory_pub.publish(trajectory_msg);

  // Publish waypoint for manipulator joints
  trajectory_msgs::JointTrajectory joint_trajectory_msg;
  joint_trajectory_msg.header.stamp = ros::Time::now();
  Eigen::VectorXd desired_joints_angle(3);
  desired_joints_angle << 0, 3*M_PI/4, M_PI/4;
  manipulator_msgs::msgJointTrajectoryFromAngles(desired_joints_angle, &joint_trajectory_msg);

  ROS_INFO("Publishing manipulator joints waypoint on namespace %s: [%f, %f, %f].",
           nh.getNamespace().c_str(),
           desired_joints_angle.x(),
           desired_joints_angle.y(),
           desired_joints_angle.z());
  joint_trajectory_pub.publish(joint_trajectory_msg);

  ros::spin();
}
