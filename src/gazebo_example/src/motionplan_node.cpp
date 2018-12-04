#include "gazebo_example/motionplan_node.h"

namespace gazebo_example
{
  
const unsigned int MotionplanNode::MAX_PUB_QUEUE = 10;
const unsigned int MotionplanNode::MAX_SUB_QUEUE = 10;

  
typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;

MotionplanNode::MotionplanNode():
  start_(false)
  , node_handle("")
{

}

bool MotionplanNode::init()
{
  init_pose_pub_ = node_handle.advertise<geometry_msgs::Pose>("init_pose", MAX_PUB_QUEUE);
  //reftwist_pub_ = node_handle.advertise<geometry_msgs::Twist>("reftwist", MAX_PUB_QUEUE);
  trajectory_pub_ = node_handle.advertise<std_msgs::Int16>("trajectory", MAX_PUB_QUEUE);

  //traj_pcl_pub_ = node_handle.advertise<pcl::PointCloud<pcl::PointXYZ>> ("traj_pcl", 1);

  pose_sub_ = node_handle.subscribe("pose", MAX_SUB_QUEUE, 
                                    &MotionplanNode::poseCallback, this);
  twist_sub_ = node_handle.subscribe("twist", MAX_SUB_QUEUE, 
                                     &MotionplanNode::twistCallback, this);
  points_sub_ = node_handle.subscribe<pcl::PointCloud<pcl::PointXYZ>>("points", 1, &MotionplanNode::callback, this);
  start_service_ = node_handle.advertiseService("start_motionplan",
                                                &MotionplanNode::start_motionplan,
                                                this);
  vis_pub = node_handle.advertise<visualization_msgs::Marker>( "visualization_marker", 0 );

  return true;
}

void MotionplanNode::run()
{
  wait_for_trigger();
  
  const double frequency = 4.0; 
  ros::Rate loop_rate(frequency);
  
  while(ros::ok())
  {
    // Handle Callbacks
    ros::spinOnce();
    
    // Compute and publish controller output
    compute_refstate();
    init_pose_pub_.publish(init_pose_);
    trajectory_pub_.publish(trajectory_);

    pcl_conversions::toPCL(ros::Time::now(), traj_pcl_ptr_->header.stamp);

    //traj_pcl_pub_.publish(traj_pcl_ptr_);

    
    // Wait to maintain constant frequency
    loop_rate.sleep();
  }
}

void MotionplanNode::wait_for_trigger()
{
  while(ros::ok())
  {
    ros::spinOnce();
    if(start_) {break;}
    ROS_INFO_THROTTLE(10, "Waiting for motionplan start trigger");
  }
}

void MotionplanNode::compute_refstate()
{
  gazebo::math::Vector3 p_i(pose_.position.x, pose_.position.y, pose_.position.z);
  gazebo::math::Quaternion q(pose_.orientation.w, pose_.orientation.x, pose_.orientation.y, pose_.orientation.z);
  pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> octree = pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> (.1);//resolution is 128,.1
  octree.setInputCloud (cloud);
  octree.addPointsFromInputCloud ();


  //trajectory_.data = Traj_Lib.SelectTrajectory2(octree,q,p_i);
  trajectory_.data = 1;
  std::vector<node> nodes = Traj_Lib.SelectTrajectories(octree,q,p_i,p_i);
  printf("%i\n", nodes.size());

  visualization_msgs::Marker marker;
  marker.header.frame_id = "camera_link";
  marker.header.stamp = ros::Time();
  marker.ns = "my_namespace";
  marker.id = 0;
  marker.type = visualization_msgs::Marker::SPHERE;
  marker.action = visualization_msgs::Marker::ADD;
  marker.pose.position.x = 1;
  marker.pose.position.y = 1;
  marker.pose.position.z = 1;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;
  marker.scale.x = 1;
  marker.scale.y = 0.1;
  marker.scale.z = 0.1;
  marker.color.a = 1.0; // Don't forget to set the alpha!
  marker.color.r = 0.0;
  marker.color.g = 1.0;
  marker.color.b = 0.0;

vis_pub.publish( marker );
  //printf("%i\n", nodes[0].GetSortedManeuvers()[0]);


  //trajectory_.data = Traj_Lib.SelectTrajectory(0.5,octree,q);
//Traj_Lib.DistanceToObstacle(octree,q);
//Traj_Lib.DistanceToGlobalLine(p_i);
//Traj_Lib.Cost(octree,q,p_i);
//Traj_Lib.printC();
  /*Traj_Lib.DistanceToGlobalLine(p_i);
  Traj_Lib.DistanceToObstacle(octree,q);
  double C1 = 1000.0;
  double C2 = 50.0;
  double d_crash = 0.5;
  double d_far = 3.0;
  double cost;

  for (int i = 0; i < Traj_Lib.GetNumberOfTrajectories(); ++i){
    if (Traj_Lib.d_obstacle[i] < d_crash){
      cost = C1 + Traj_Lib.d_global[i];
    }
    else if (Traj_Lib.d_obstacle[i] < d_far){
      cost = C2 * (Traj_Lib.d_obstacle[i] - d_crash) + Traj_Lib.d_global[i];
    }
    else{
      cost = Traj_Lib.d_global[i];
    }

    Traj_Lib.C.push_back(cost);
  }

  double lowest_cost = Traj_Lib.C[0];
  int best_trajectory = 0;

  for (int i = 0; i < Traj_Lib.GetNumberOfTrajectories(); ++i){
    printf("%f\n",Traj_Lib.d_obstacle[i] );
    printf("%f\n",Traj_Lib.C[i] );
    if (Traj_Lib.C[i] < lowest_cost){
      lowest_cost = Traj_Lib.C[i];
      best_trajectory = i;
    }
  } 
  trajectory_.data = best_trajectory;*/
  /*traj_pcl_ptr_->header.frame_id = "plane_xacro_default/camera_link";
  //traj_pcl_ptr_->height = traj_pcl_ptr_->width = 1;
  traj_pcl_ptr_->points.clear();
  for (int j = 0; j<Traj_Lib.GetNumberOfTrajectories();){

  
  for (int i = 0; i < Traj_Lib.GetTrajectoryAtIndex(j).number_of_lines; i++){
    gazebo::math::Vector3 pos = Traj_Lib.GetTrajectoryAtIndex(j).TransformPointToCameraFrame(q, i);
   

  traj_pcl_ptr_->points.push_back (pcl::PointXYZ(pos[0], pos[1], pos[2]));
  }
  j= j + 1;
  }*/

  /*traj_pcl_ptr_->header.frame_id = "plane_xacro_default/camera_link";
  //traj_pcl_ptr_->height = traj_pcl_ptr_->width = 1;
  traj_pcl_ptr_->points.clear(); 
  for (int i = 0; i < nodes.size(); i++){
    gazebo::math::Vector3 pos = nodes[i].GetPosition();
    traj_pcl_ptr_->points.push_back (pcl::PointXYZ(pos[0], pos[1], pos[2]));
  }*/
  
  init_pose_ = pose_;
  
}


void MotionplanNode::poseCallback(const geometry_msgs::Pose::ConstPtr& msg)
{
  pose_ = *msg;
}

void MotionplanNode::twistCallback(const geometry_msgs::Twist::ConstPtr& msg)
{
  twist_ = *msg;
}

void MotionplanNode::callback(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& msg)
{
  points_ = *msg;


  //printf ("Cloud: width = %d, height = %d\n", msg->width, msg->height);
  //BOOST_FOREACH (const pcl::PointXYZ& pt, msg->points)
  //printf ("\t(%f, %f, %f)\n", pt.x, pt.y, pt.z);
}

bool gazebo_example::MotionplanNode::start_motionplan(std_srvs::Trigger::Request& req,
                                                      std_srvs::Trigger::Response& res)
{
  start_ = true;
  res.success = true;
  /*p_0.x = pose_.position.x;
  p_0.y = -pose_.position.y;
  p_0.z = -pose_.position.z;
  p_final.x = 100.0;
  p_final.y = 0.0;
  p_final.z = -5.0;
  psi_global = atan((p_final.y-p_0.y)/(p_final.x-p_0.x));
  u_global = 5.0;
  theta_global = 0.0191*u_global*u_global - 0.3022*u_global + 1.3262;
  q_global.SetFromEuler({0.0,-theta_global,-psi_global});*/
  /*cloud->width = 1280;
  cloud->height = 720;
  //kdtree.setInputCloud (cloud);
  octree.setInputCloud (cloud);
  octree.addPointsFromInputCloud ();*/

  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_0.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_15.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-15.csv");
  /*filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_30.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-30.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_45.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-45.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_60.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-60.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_0_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_15_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-15_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_30_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-30_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_45_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-45_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_60_extended.csv");
  filenames.push_back("/home/eitan/mcfoamy_gazebo/src/gazebo_example/include/gazebo_example/trajectory_csvs/7_-60_extended.csv");*/
  Traj_Lib.LoadLibrary(filenames);
  gazebo::math::Vector3 p_i(pose_.position.x, pose_.position.y, -10.0);
  gazebo::math::Quaternion q(pose_.orientation.w, pose_.orientation.x, pose_.orientation.y, pose_.orientation.z);
  //Traj_Lib.SetGlobalLine(p_i, q.GetYaw());



}

} // gazebo_example namespace



int main(int argc, char **argv)
{
  ros::init(argc, argv, "motionplan_node");
  
  gazebo_example::MotionplanNode node;
  if(!node.init())
  {
    return 0;
  }
  
  node.run();

  return 0;
}
