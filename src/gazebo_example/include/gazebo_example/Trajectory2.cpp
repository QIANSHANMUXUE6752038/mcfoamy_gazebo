/*
 * Trajectory class representing a trajectory in state space
 *
 * Author: Eitan Bulka, <eitan.bulka@mail.mcgill.ca> 2018
 *
 */

#include "Trajectory2.hpp"


Trajectory::Trajectory(std::string filename_csv) {
    trajectory_number = -1;
    LoadTrajectory(filename_csv, data);
    dt = data(1,0) - data(0,0); //assume constant delta t in trajectory
    aircraft_geometry.push_back(gazebo::math::Vector3(0.1,0.0,0.0));
    aircraft_geometry.push_back(gazebo::math::Vector3(0.0,0.5,0.0));
    aircraft_geometry.push_back(gazebo::math::Vector3(0.0,-0.5,0.0));
    aircraft_geometry.push_back(gazebo::math::Vector3(-0.7,0.0,0.0));

  	d_min = 0.2;
    d_max = 1.0*2.0;
    max_speed = 10.0;

    lambda = 0.0;
    //C_cb = gazebo::math::Matrix3(0.0,-sin(lambda),cos(lambda),1.0,0.0,0.0,0.0,cos(lambda),sin(lambda));
    C_cb = gazebo::math::Matrix3(0.0, 1.0, 0.0, -sin(lambda), 0.0, cos(lambda), cos(lambda), 0.0, sin(lambda));

}


void Trajectory::LoadTrajectory(const std::string& filename, Eigen::MatrixXd &matrix) {
    SetNumberOfLines(filename);
    matrix.resize(number_of_lines,14); // minus 1 for header, i = number of columns

    int i =  0;
    int row_num = 0;

    CsvParser *csvparser = CsvParser_new(filename.c_str(), ",", 0);
    CsvRow *row;

    while ((row = CsvParser_getRow(csvparser)) ) {
      //printf("==NEW LINE==\n");
        const char **rowFields = CsvParser_getFields(row);
        for (i = 0 ; i < CsvParser_getNumFields(row) ; i++) {
            //printf("FIELD: %s\n", rowFields[i]);
            matrix(row_num,i) = atof(rowFields[i]);

        }
    //printf("\n");
        CsvParser_destroy_row(row);
        row_num ++;
    }
    CsvParser_destroy(csvparser);

}


void Trajectory::SetNumberOfLines(const std::string& filename) {
    number_of_lines = 0;
    std::string line;
    std::ifstream myfile(filename.c_str());

    while (getline(myfile, line)) {
        ++number_of_lines;
    }

}

int Trajectory::GetNumberOfLines(){
  return number_of_lines;
}

Eigen::VectorXd Trajectory::GetStateAtIndex(int index){
 	return data.row(index);
}

int Trajectory::GetIndexAtTime(double time){
  int index = int(time/dt);
  if (index > number_of_lines-1){
    index = number_of_lines-1;
  }
  return index;
}

Eigen::VectorXd Trajectory::GetStateAtTime(double time){
  return GetStateAtIndex(GetIndexAtTime(time));
}



gazebo::math::Vector3 Trajectory::TransformPointToCameraFrame(gazebo::math::Quaternion q, double yaw_offset, gazebo::math::Vector3 position_offset_i, int index){
  gazebo::math::Matrix3 C_yaw_offset(cos(yaw_offset),-sin(yaw_offset),0.0,sin(yaw_offset),cos(yaw_offset),0.0,0.0,0.0,1.0);//rotate by yaw offset
  Eigen::VectorXd state = GetStateAtIndex(index);
  gazebo::math::Vector3 p_yaw_offset(state(1),state(2),state(3));
  gazebo::math::Vector3 p_c = C_cb * q.GetAsMatrix3().Inverse() * (C_yaw_offset * p_yaw_offset + position_offset_i);
  return p_c;
}

double Trajectory::DistanceToObstacle(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, gazebo::math::Quaternion q, double yaw_offset, gazebo::math::Vector3 position_offset_i)
{
  pcl::PointXYZ searchPoint;
  std::vector<int> closest_point_index(1);
  std::vector<float> closest_distance_squared(1);
  double distance = 100.0;

  int i = 0;
  searchPoint.x = 0.0;
  searchPoint.y = 0.0;
  searchPoint.z = 0.0;
  if (octree.getLeafCount() > 0){
    if (octree.nearestKSearch (searchPoint, 1, closest_point_index, closest_distance_squared) > 0){
      while(i < number_of_lines){

        p_c = TransformPointToCameraFrame(q,yaw_offset,position_offset_i,i); 
        searchPoint.x = p_c[0];
        searchPoint.y = p_c[1];
        searchPoint.z = p_c[2];

        octree.nearestKSearch (searchPoint, 1, closest_point_index, closest_distance_squared);
        if (sqrt(closest_distance_squared[0]) < distance){
          distance = sqrt(closest_distance_squared[0]);
        }
        // Don't keep searching for min distance because this trajectory is crashing anyways
        if (sqrt(closest_distance_squared[0]) < d_min/2.0){
          break;
        }/*else if (sqrt(closest_distance_squared[0]) < d_max/2.0){
          Eigen::VectorXd state = GetStateAtIndex(i);
          gazebo::math::Vector3 p_psi(state(1),state(2),state(3));
          gazebo::math::Matrix3 C_psi(cos(q.GetYaw()),-sin(q.GetYaw()),0.0,sin(q.GetYaw()),cos(q.GetYaw()),0.0,0.0,0.0,1.0);//rotate by negative yaw, but because gazebo uses forward-left-up, GetYaw function returns negative yaw

          gazebo::math::Quaternion q_r(state(4),state(5),state(6),state(7));
          for (int ii = 0; ii < aircraft_geometry.size(); ++i){
            gazebo::math::Vector3 new_search_point = C_cb * q.GetAsMatrix3() * C_psi * (q_r.GetAsMatrix3().Inverse() * aircraft_geometry[i] + p_psi);
            searchPoint.x = new_search_point[0];
            searchPoint.y = new_search_point[1];
            searchPoint.z = new_search_point[2];
            octree.nearestKSearch (searchPoint, 1, closest_point_index, closest_distance_squared);
            if (sqrt(closest_distance_squared[0]) < distance){
              distance = sqrt(closest_distance_squared[0]);
            }
          }
        }*/else
        {
          i = i + 1;
          //double t_free = sqrt(closest_distance_squared[0])/max_speed;
          //i = i + int(t_free / dt); // assuming fixed time step in trajectory
        }
        // Checking distance using orientation of aircraft because distance is less than wingspan/2
        /*if (sqrt(closest_distance_squared[0]) > d_min/2.0 && sqrt(closest_distance_squared[0]) < d_max/2.0){
          //use Vector3/Matrix3 to C_ib * aircraft geometry, find min distance

        }

        if (sqrt(closest_distance_squared[0]) > d_max/2.0){
          double t_free = sqrt(closest_distance_squared[0])/max_speed;
          i = i + int(t_free / dt); // assuming fixed time step in trajectory
        }*/

      }
    }
  }

  return distance;
  
}
bool Trajectory::NoCollision(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, gazebo::math::Quaternion q, double yaw_offset, gazebo::math::Vector3 position_offset_i){
  // if implemented could make faster by stopping distancetoobstacle when reaching collision distance.
  double distance_to_obstacle = DistanceToObstacle(octree,q,yaw_offset,position_offset_i);
  if (distance_to_obstacle < d_max/2.0){
    return false;
  }
  else
    return true;
}

double Trajectory::DistanceToIntermediateGoal(double yaw_offset, gazebo::math::Vector3 position_offset_i, gazebo::math::Vector3 intermediate_goal_i){
  return (intermediate_goal_i - End_Position(yaw_offset, position_offset_i)).GetLength();
}

double Trajectory::YawDistanceToGoal(double yaw_offset, gazebo::math::Vector3 p_i, gazebo::math::Vector3 p_goal_i){
  //double yaw_distance = atan2((p_goal_i - p_i).y , (p_goal_i - p_i).x) - End_Yaw(yaw_offset);
  double yaw_distance = atan2((p_goal_i - p_i).y , (p_goal_i - p_i).x) - End_Quaternion(yaw_offset).GetYaw();

  if (yaw_distance > 3.14159265){
    yaw_distance = yaw_distance - 2.0 * 3.14159265;
  }
  if (yaw_distance < -3.14159265){
    yaw_distance = yaw_distance + 2.0 * 3.14159265;
  }

  return fabs(yaw_distance);
  
}

double Trajectory::AngleDistanceToGoal(double yaw_offset, gazebo::math::Vector3 p_initial_i, gazebo::math::Vector3 p_goal_i){
  gazebo::math::Vector3 f_hat_i = End_Quaternion(yaw_offset).GetAsMatrix3() * gazebo::math::Vector3(1.0, 0.0, 0.0);
  return acos(f_hat_i.Dot((p_goal_i - p_initial_i).Normalize()));
}

double Trajectory::AngleDistanceToGoal2(double yaw_offset, gazebo::math::Vector3 position_offset_i, gazebo::math::Vector3 p_initial_i, gazebo::math::Vector3 p_goal_i){
  gazebo::math::Vector3 relative_end_position_i = End_Position(yaw_offset, position_offset_i) - p_initial_i;
  double climb_angle = atan(-relative_end_position_i.z / sqrt(pow(relative_end_position_i.x,2.0) + pow(relative_end_position_i.y,2.0)));
  gazebo::math::Vector3 g_hat_i = gazebo::math::Quaternion(0.0, climb_angle, End_Yaw(yaw_offset)).GetAsMatrix3() * gazebo::math::Vector3(1.0, 0.0, 0.0);
  return acos(g_hat_i.Dot((p_goal_i - p_initial_i).Normalize()));
}

double Trajectory::DistanceToGoal(double yaw_offset, gazebo::math::Vector3 position_offset_i, gazebo::math::Vector3 p_initial_i, gazebo::math::Vector3 p_goal_i){
  double yaw_distance = YawDistanceToGoal(yaw_offset, p_initial_i, p_goal_i);
  double z_distance = fabs((p_goal_i - End_Position(yaw_offset, position_offset_i)).z);
  return yaw_distance/3.14159265 + z_distance/4.0;

}

gazebo::math::Vector3 Trajectory::End_Position(double yaw_offset, gazebo::math::Vector3 position_offset_i){
  gazebo::math::Matrix3 C_yaw_offset(cos(yaw_offset),-sin(yaw_offset),0.0,sin(yaw_offset),cos(yaw_offset),0.0,0.0,0.0,1.0);//rotate by yaw offset
  Eigen::VectorXd final_state = GetStateAtIndex(number_of_lines-1);
  gazebo::math::Vector3 p_yaw_offset(final_state(1),final_state(2),final_state(3));
  return C_yaw_offset * p_yaw_offset + position_offset_i;
}

gazebo::math::Vector3 Trajectory::GetPosition(double yaw_offset, gazebo::math::Vector3 position_offset_i, double t){
  gazebo::math::Matrix3 C_yaw_offset(cos(yaw_offset),-sin(yaw_offset),0.0,sin(yaw_offset),cos(yaw_offset),0.0,0.0,0.0,1.0);//rotate by yaw offset
  Eigen::VectorXd final_state = GetStateAtTime(t);
  gazebo::math::Vector3 p_yaw_offset(final_state(1),final_state(2),final_state(3));
  return C_yaw_offset * p_yaw_offset + position_offset_i;
}

double Trajectory::End_Yaw(double yaw_offset){
  Eigen::VectorXd final_state = GetStateAtIndex(number_of_lines-1);
  gazebo::math::Quaternion q_final(final_state(4),final_state(5),final_state(6),final_state(7));
  double end_yaw = yaw_offset + q_final.GetYaw();
  if (end_yaw > 3.14159265){
    end_yaw = end_yaw - 2.0 * 3.14159265;
  }


  if (end_yaw < -3.14159265){
    end_yaw = end_yaw + 2.0 * 3.14159265;
  }
  return end_yaw;
}

gazebo::math::Quaternion Trajectory::End_Quaternion(double yaw_offset){
  Eigen::VectorXd final_state = GetStateAtIndex(number_of_lines-1);
  gazebo::math::Quaternion q_yaw_offset(0.0, 0.0, yaw_offset);
  gazebo::math::Quaternion q_final_no_offset(final_state(4),final_state(5),final_state(6),final_state(7));
  gazebo::math::Quaternion q_final = q_yaw_offset * q_final_no_offset;
  /*gazebo::math::Quaternion q_final2(q_final_no_offset.GetRoll(), q_final_no_offset.GetPitch(), q_final_no_offset.GetYaw() + yaw_offset);
  printf("%f\n", q_final.x - q_final2.x);
  printf("%f\n", q_final.y - q_final2.y);
  printf("%f\n", q_final.z - q_final2.z);
  printf("%f\n", q_final.w - q_final2.w);*/

  return q_final;
}

gazebo::math::Quaternion Trajectory::GetQuaternion(double yaw_offset, double t){
  Eigen::VectorXd state = GetStateAtTime(t);
  gazebo::math::Quaternion q_yaw_offset(0.0, 0.0, yaw_offset);
  gazebo::math::Quaternion q_no_offset(state(4),state(5),state(6),state(7));
  gazebo::math::Quaternion q = q_yaw_offset * q_no_offset;
  /*gazebo::math::Quaternion q_final2(q_final_no_offset.GetRoll(), q_final_no_offset.GetPitch(), q_final_no_offset.GetYaw() + yaw_offset);
  printf("%f\n", q_final.x - q_final2.x);
  printf("%f\n", q_final.y - q_final2.y);
  printf("%f\n", q_final.z - q_final2.z);
  printf("%f\n", q_final.w - q_final2.w);*/

  return q;
}

bool Trajectory::InFieldOfView(gazebo::math::Quaternion q, double yaw_offset, gazebo::math::Vector3 position_offset_i){
  gazebo::math::Vector3 p_c = TransformPointToCameraFrame(q, yaw_offset, position_offset_i, number_of_lines-1);
  bool out = true;
  double HFOV = 85.2*3.14/180.0;//for realsense d435
  double VFOV = 58.0*3.14/180.0;//for realsense d435
  double range = 10.0; //for realsense d435
  if (fabs(atan2(p_c.x,p_c.z)) > HFOV/2.0){
    out = false;
  }

  if (fabs(atan2(p_c.y,p_c.z)) > VFOV/2.0){
    out = false;
  }
  if (p_c.GetLength() > range){
    out = false;
  }
  return out;
}


node::node(){

}

void node::SetParent(int x){
  parent = x;
}

int node::GetParent(){
  return parent;
}

void node::SetIndex(int x){
  index = x;
}

int node::GetIndex(){
  return index;
}

void node::SetSortedManeuvers(std::vector<size_t> x){
  sorted_maneuvers = x;
}

std::vector<size_t> node::GetSortedManeuvers(){
  return sorted_maneuvers;
}

void node::SetPosition(gazebo::math::Vector3 x){
  position_i = x;
}

gazebo::math::Vector3 node::GetPosition(){
  return position_i;
}

void node::SetYaw(double x){
  yaw = x;
}

double node::GetYaw(){
  return yaw;
}

TrajectoryLibrary::TrajectoryLibrary(){
  lambda = 0.0;
  C_cb = gazebo::math::Matrix3(0.0, 1.0, 0.0, -sin(lambda), 0.0, cos(lambda), cos(lambda), 0.0, sin(lambda));

}

void TrajectoryLibrary::LoadLibrary(std::vector<std::string> filenames){
  number_of_trajectories = filenames.size();
  for (int i = 0; i < number_of_trajectories; ++i){
    trajectory_library.push_back(Trajectory(filenames[i]));
  }
}

Trajectory TrajectoryLibrary::GetTrajectoryAtIndex(int index){
  return trajectory_library[index];
}

int TrajectoryLibrary::GetNumberOfTrajectories(){
  return number_of_trajectories;
}

std::vector<node> TrajectoryLibrary::SelectTrajectories(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, gazebo::math::Quaternion q,gazebo::math::Vector3 p_initial_i,gazebo::math::Vector3 p_goal_i){
  std::vector<node> nodes;
  node q_start;
  q_start.SetPosition(p_initial_i);
  q_start.SetYaw(q.GetYaw());
  q_start.SetParent(-1);
  size_t index = 0;
  size_t number_of_nodes = 1;
  q_start.SetIndex(index);

  //compute intermediate goal. For now only in 2D.
  gazebo::math::Vector3 p_rel_goal_c = C_cb * q.GetAsMatrix3().Inverse() * (p_goal_i - p_initial_i);
  double HFOV = 85.2*3.14/180.0;//for realsense d435
  double VFOV = 58.0*3.14/180.0;//for realsense d435
  double range = 10.0; //for realsense d435

  double gamma = atan2(p_rel_goal_c.x,p_rel_goal_c.z);
  if (gamma > HFOV/2.0){
    gazebo::math::Vector3 relative_intermediate_goal_c(range * sin(HFOV/2.0), range * cos(HFOV/2.0),0.0);
  }
  else if (gamma < -HFOV/2.0){
    gazebo::math::Vector3 relative_intermediate_goal_c(range * sin(-HFOV/2.0), range * cos(-HFOV/2.0),0.0);
  }
  else {
    gazebo::math::Vector3 relative_intermediate_goal_c(range * sin(gamma), range * cos(gamma),0.0);
  }
  gazebo::math::Vector3 intermediate_goal_i = q.GetAsMatrix3() * C_cb.Inverse() * p_rel_goal_c + p_initial_i;

  std::vector<double> distances_to_goal;
  for (int i = 0; i < number_of_trajectories; ++i){
    distances_to_goal.push_back(GetTrajectoryAtIndex(i).DistanceToIntermediateGoal(q_start.GetYaw(),q_start.GetPosition(), intermediate_goal_i));
  } 

  std::vector<size_t> sorted_distances_to_goal_indices;
  std::vector<double> sorted_distances_to_goal;
  sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
  q_start.SetSortedManeuvers(sorted_distances_to_goal_indices);
  nodes.push_back(q_start);

  std::vector<size_t> indices;
  size_t deleted_maneuvers;

  node q_curr = q_start;
  double tolerance = 3.0;
  bool nobreak;
  node q_new;

  while ((q_curr.GetPosition() - intermediate_goal_i).GetLength() > tolerance){
    indices.push_back(index);
    deleted_maneuvers = 0;
    nobreak = true;


    for (size_t i = 0; i < q_curr.GetSortedManeuvers().size(); ++i){     
      if (GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).NoCollision(octree,q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i) && GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).InFieldOfView(q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i)){
        //make new node
        q_new.SetParent(index);
        q_new.SetIndex(number_of_nodes);
        q_new.SetPosition(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).End_Position(q_curr.GetYaw(),q_curr.GetPosition()));
        distances_to_goal.clear();
        sorted_distances_to_goal.clear();
        sorted_distances_to_goal_indices.clear();
        for (int j = 0; j < number_of_trajectories; ++j){
          distances_to_goal.push_back(GetTrajectoryAtIndex(j).DistanceToIntermediateGoal(q_new.GetYaw(),q_new.GetPosition(), intermediate_goal_i));
        }
        sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
        q_new.SetSortedManeuvers(sorted_distances_to_goal_indices);
        q_new.SetYaw(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).End_Yaw(q_curr.GetYaw()));
        nodes.push_back(q_new);
        ++number_of_nodes;
        index = q_new.GetIndex();
        q_curr = q_new;
        nobreak = false;
        break; 
      }
      else{
        //q_curr.SetSortedManeuvers(q_curr.GetSortedManeuvers().erase(q_curr.GetSortedManeuvers().begin() + i-deleted_maneuvers));
        std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
        temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin() + i-deleted_maneuvers);
        q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
        ++deleted_maneuvers;
        nodes[index] = q_curr;
      }
    }
    if (nobreak){
      index = q_curr.GetParent();
      if (index == -1){
        break;
      }
      q_curr = nodes[index];
      //q_curr.SetSortedManeuvers(q_curr.GetSortedManeuvers().erase(q_curr.GetSortedManeuvers().begin()));
      std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
      temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin());
      q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
      nodes[index] = q_curr;
    }
    if (indices.size()>1000){
      printf("%i\n", indices.size());
      break;
    } 


  }

  std::vector<node> final_nodes;
  std::vector<int> final_nodes_index;
  int i = nodes.size()-1;
  while (nodes[i].GetParent() > -1){
    final_nodes_index.push_back(nodes[i].GetIndex());
    i = nodes[i].GetParent();
  }
  final_nodes.push_back(nodes[0]);
  for (int i = final_nodes_index.size() - 1; i > -1; --i){
    final_nodes.push_back(nodes[final_nodes_index[i]]);
  }
  //return final_nodes;
  return nodes;
}

void TrajectoryLibrary::SelectTrajectories2(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, gazebo::math::Quaternion q,gazebo::math::Vector3 p_initial_i,gazebo::math::Vector3 p_goal_i, std::vector<node> *nodes_out, std::vector<node> *final_nodes_out, gazebo::math::Vector3 *intermediate_goal_i_out){
  std::vector<node> nodes;
  node q_start;
  q_start.SetPosition(p_initial_i);
  q_start.SetYaw(q.GetYaw());
  q_start.SetParent(-1);
  size_t index = 0;
  size_t number_of_nodes = 1;
  q_start.SetIndex(index);

  //compute intermediate goal. For now only in 2D.
  gazebo::math::Vector3 p_rel_goal_c = C_cb * q.GetAsMatrix3().Inverse() * (p_goal_i - p_initial_i);
  double HFOV = 85.2*3.14/180.0;//for realsense d435
  double VFOV = 58.0*3.14/180.0;//for realsense d435
  double range = 10.0; //for realsense d435
  gazebo::math::Vector3 relative_intermediate_goal_c;
  double gamma = atan2(p_rel_goal_c.x,p_rel_goal_c.z);
  if (gamma > 0.9*(HFOV/2.0)){
    gamma = 0.9*(HFOV/2.0);
  }
  if (gamma < 0.9*(-HFOV/2.0)){
    gamma = 0.9*(-HFOV/2.0);
  }

  relative_intermediate_goal_c.x = 0.9 * range * sin(gamma);
  relative_intermediate_goal_c.y = 0.0;
  relative_intermediate_goal_c.z = 0.9 * range * cos(gamma);
  
  gazebo::math::Vector3 intermediate_goal_i = q.GetAsMatrix3() * C_cb.Inverse() * relative_intermediate_goal_c + p_initial_i;

  std::vector<double> distances_to_goal;
  for (int i = 0; i < number_of_trajectories; ++i){
    distances_to_goal.push_back(GetTrajectoryAtIndex(i).DistanceToIntermediateGoal(q_start.GetYaw(),q_start.GetPosition(), intermediate_goal_i));
  } 

  std::vector<size_t> sorted_distances_to_goal_indices;
  std::vector<double> sorted_distances_to_goal;
  sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
  q_start.SetSortedManeuvers(sorted_distances_to_goal_indices);
  nodes.push_back(q_start);

  std::vector<size_t> indices;
  size_t deleted_maneuvers;

  node q_curr = q_start;
  double tolerance = 3.0;
  bool nobreak;
  node q_new;

  while ((q_curr.GetPosition() - intermediate_goal_i).GetLength() > tolerance){
    indices.push_back(index);
    deleted_maneuvers = 0;
    nobreak = true;


    for (size_t i = 0; i < q_curr.GetSortedManeuvers().size(); ++i){     
      if (GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).NoCollision(octree,q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i) && GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).InFieldOfView(q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i)){
        //make new node
        q_new.SetParent(index);
        q_new.SetIndex(number_of_nodes);
        q_new.SetPosition(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).End_Position(q_curr.GetYaw(),q_curr.GetPosition()));
        distances_to_goal.clear();
        sorted_distances_to_goal.clear();
        sorted_distances_to_goal_indices.clear();
        for (int j = 0; j < number_of_trajectories; ++j){
          distances_to_goal.push_back(GetTrajectoryAtIndex(j).DistanceToIntermediateGoal(q_new.GetYaw(),q_new.GetPosition(), intermediate_goal_i));
        }
        sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
        q_new.SetSortedManeuvers(sorted_distances_to_goal_indices);
        q_new.SetYaw(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[i-deleted_maneuvers]).End_Yaw(q_curr.GetYaw()));
        nodes.push_back(q_new);
        ++number_of_nodes;
        index = q_new.GetIndex();
        q_curr = q_new;
        nobreak = false;
        break; 
      }
      else{
        //q_curr.SetSortedManeuvers(q_curr.GetSortedManeuvers().erase(q_curr.GetSortedManeuvers().begin() + i-deleted_maneuvers));
        std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
        temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin() + i-deleted_maneuvers);
        q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
        ++deleted_maneuvers;
        nodes[index] = q_curr;
      }
    }
    if (nobreak){
      index = q_curr.GetParent();
      if (index == -1){
        break;
      }
      q_curr = nodes[index];
      //q_curr.SetSortedManeuvers(q_curr.GetSortedManeuvers().erase(q_curr.GetSortedManeuvers().begin()));
      std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
      temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin());
      q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
      nodes[index] = q_curr;
    }
    if (indices.size()>1000){
      printf("%i\n", indices.size());
      break;
    } 


  }

  std::vector<node> final_nodes;
  std::vector<int> final_nodes_index;
  int i = nodes.size()-1;
  while (nodes[i].GetParent() > -1){
    final_nodes_index.push_back(nodes[i].GetIndex());
    i = nodes[i].GetParent();
  }
  final_nodes.push_back(nodes[0]);
  for (int i = final_nodes_index.size() - 1; i > -1; --i){
    final_nodes.push_back(nodes[final_nodes_index[i]]);
  }

  *nodes_out = nodes;
  *final_nodes_out = final_nodes;
  *intermediate_goal_i_out = intermediate_goal_i;

}

void TrajectoryLibrary::SelectTrajectories3(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> &octree, gazebo::math::Quaternion q,gazebo::math::Vector3 p_initial_i,gazebo::math::Vector3 p_goal_i, std::vector<node> *nodes_out, std::vector<node> *final_nodes_out){
  std::vector<node> nodes;
  node q_start;
  q_start.SetPosition(p_initial_i);
  q_start.SetYaw(q.GetYaw());
  q_start.SetParent(-1);
  size_t index = 0;
  size_t number_of_nodes = 1;
  q_start.SetIndex(index);


  std::vector<double> distances_to_goal;
  for (int i = 0; i < number_of_trajectories; ++i){
    distances_to_goal.push_back(GetTrajectoryAtIndex(i).DistanceToGoal(q_start.GetYaw(),q_start.GetPosition(),p_initial_i, p_goal_i));
  } 

  std::vector<size_t> sorted_distances_to_goal_indices;
  std::vector<double> sorted_distances_to_goal;
  sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
  q_start.SetSortedManeuvers(sorted_distances_to_goal_indices);
  nodes.push_back(q_start);

  std::vector<size_t> indices;

  node q_curr = q_start;
  double tolerance = 3.0;
  bool nobreak;
  node q_new;
  bool loop_on = true;

  while (loop_on){
    indices.push_back(index);
    nobreak = true;

    //while (q_curr.GetSortedManeuvers().size() > 0){ 
    for (size_t i = 0; i < q_curr.GetSortedManeuvers().size(); ++i){  
      if (GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[0]).NoCollision(octree,q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i)){
        if (GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[0]).InFieldOfView(q,q_curr.GetYaw(),q_curr.GetPosition() - p_initial_i)){
          //make new node
          q_new.SetParent(index);
          q_new.SetIndex(number_of_nodes);
          q_new.SetPosition(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[0]).End_Position(q_curr.GetYaw(),q_curr.GetPosition()));
          q_new.SetYaw(GetTrajectoryAtIndex(q_curr.GetSortedManeuvers()[0]).End_Yaw(q_curr.GetYaw()));
          distances_to_goal.clear();
          sorted_distances_to_goal.clear();
          sorted_distances_to_goal_indices.clear();
          for (int j = 0; j < number_of_trajectories; ++j){
            distances_to_goal.push_back(GetTrajectoryAtIndex(j).DistanceToGoal(q_new.GetYaw(), q_new.GetPosition(), p_initial_i, p_goal_i));
          }
          sort(distances_to_goal,sorted_distances_to_goal,sorted_distances_to_goal_indices);
          q_new.SetSortedManeuvers(sorted_distances_to_goal_indices);
          nodes.push_back(q_new);
          ++number_of_nodes;
          index = q_new.GetIndex();
          q_curr = q_new;
        }
        else{
          //this trajectory took us out of field of view so last new node
          loop_on = false;
        }
        nobreak = false;
        break; 
      }
      else{
        std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
        temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin());
        q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
        nodes[index] = q_curr;
      }
    }
    if (nobreak){
      index = q_curr.GetParent();
      if (index == -1){
        break;
      }
      q_curr = nodes[index];
      std::vector<size_t> temp_sorted_maneuvers = q_curr.GetSortedManeuvers();
      temp_sorted_maneuvers.erase(temp_sorted_maneuvers.begin());
      q_curr.SetSortedManeuvers(temp_sorted_maneuvers);
      nodes[index] = q_curr;
    }



  }


  /*std::vector<node> final_nodes;
  std::vector<int> final_nodes_index;
  int i = nodes.size()-1;
  while (nodes[i].GetParent() > -1){
    final_nodes_index.push_back(nodes[i].GetIndex());
    i = nodes[i].GetParent();
  }
  final_nodes.push_back(nodes[0]);
  for (int i = final_nodes_index.size() - 1; i > -1; --i){
    final_nodes.push_back(nodes[final_nodes_index[i]]);
  }*/

  std::vector<node> final_nodes;
  int i = index;
  while (i > -1){
    final_nodes.push_back(nodes[i]);
    i = nodes[i].GetParent();
  }


  *nodes_out = nodes;
  *final_nodes_out = final_nodes;

}