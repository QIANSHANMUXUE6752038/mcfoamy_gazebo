// Generated by gencpp from file gazebo_example/actuator.msg
// DO NOT EDIT!


#ifndef GAZEBO_EXAMPLE_MESSAGE_ACTUATOR_H
#define GAZEBO_EXAMPLE_MESSAGE_ACTUATOR_H


#include <string>
#include <vector>
#include <map>

#include <ros/types.h>
#include <ros/serialization.h>
#include <ros/builtin_message_traits.h>
#include <ros/message_operations.h>


namespace gazebo_example
{
template <class ContainerAllocator>
struct actuator_
{
  typedef actuator_<ContainerAllocator> Type;

  actuator_()
    : u1(0.0)
    , u2(0.0)
    , u3(0.0)
    , u4(0.0)  {
    }
  actuator_(const ContainerAllocator& _alloc)
    : u1(0.0)
    , u2(0.0)
    , u3(0.0)
    , u4(0.0)  {
  (void)_alloc;
    }



   typedef double _u1_type;
  _u1_type u1;

   typedef double _u2_type;
  _u2_type u2;

   typedef double _u3_type;
  _u3_type u3;

   typedef double _u4_type;
  _u4_type u4;





  typedef boost::shared_ptr< ::gazebo_example::actuator_<ContainerAllocator> > Ptr;
  typedef boost::shared_ptr< ::gazebo_example::actuator_<ContainerAllocator> const> ConstPtr;

}; // struct actuator_

typedef ::gazebo_example::actuator_<std::allocator<void> > actuator;

typedef boost::shared_ptr< ::gazebo_example::actuator > actuatorPtr;
typedef boost::shared_ptr< ::gazebo_example::actuator const> actuatorConstPtr;

// constants requiring out of line definition



template<typename ContainerAllocator>
std::ostream& operator<<(std::ostream& s, const ::gazebo_example::actuator_<ContainerAllocator> & v)
{
ros::message_operations::Printer< ::gazebo_example::actuator_<ContainerAllocator> >::stream(s, "", v);
return s;
}

} // namespace gazebo_example

namespace ros
{
namespace message_traits
{



// BOOLTRAITS {'IsFixedSize': True, 'IsMessage': True, 'HasHeader': False}
// {'std_msgs': ['/opt/ros/kinetic/share/std_msgs/cmake/../msg'], 'gazebo_example': ['/home/eitan/mcfoamy_gazebo/src/gazebo_example/msg']}

// !!!!!!!!!!! ['__class__', '__delattr__', '__dict__', '__doc__', '__eq__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_parsed_fields', 'constants', 'fields', 'full_name', 'has_header', 'header_present', 'names', 'package', 'parsed_fields', 'short_name', 'text', 'types']




template <class ContainerAllocator>
struct IsFixedSize< ::gazebo_example::actuator_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsFixedSize< ::gazebo_example::actuator_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::gazebo_example::actuator_<ContainerAllocator> >
  : TrueType
  { };

template <class ContainerAllocator>
struct IsMessage< ::gazebo_example::actuator_<ContainerAllocator> const>
  : TrueType
  { };

template <class ContainerAllocator>
struct HasHeader< ::gazebo_example::actuator_<ContainerAllocator> >
  : FalseType
  { };

template <class ContainerAllocator>
struct HasHeader< ::gazebo_example::actuator_<ContainerAllocator> const>
  : FalseType
  { };


template<class ContainerAllocator>
struct MD5Sum< ::gazebo_example::actuator_<ContainerAllocator> >
{
  static const char* value()
  {
    return "278bbb64bccc0a26b221d16071445863";
  }

  static const char* value(const ::gazebo_example::actuator_<ContainerAllocator>&) { return value(); }
  static const uint64_t static_value1 = 0x278bbb64bccc0a26ULL;
  static const uint64_t static_value2 = 0xb221d16071445863ULL;
};

template<class ContainerAllocator>
struct DataType< ::gazebo_example::actuator_<ContainerAllocator> >
{
  static const char* value()
  {
    return "gazebo_example/actuator";
  }

  static const char* value(const ::gazebo_example::actuator_<ContainerAllocator>&) { return value(); }
};

template<class ContainerAllocator>
struct Definition< ::gazebo_example::actuator_<ContainerAllocator> >
{
  static const char* value()
  {
    return "float64 u1\n\
float64 u2\n\
float64 u3\n\
float64 u4\n\
";
  }

  static const char* value(const ::gazebo_example::actuator_<ContainerAllocator>&) { return value(); }
};

} // namespace message_traits
} // namespace ros

namespace ros
{
namespace serialization
{

  template<class ContainerAllocator> struct Serializer< ::gazebo_example::actuator_<ContainerAllocator> >
  {
    template<typename Stream, typename T> inline static void allInOne(Stream& stream, T m)
    {
      stream.next(m.u1);
      stream.next(m.u2);
      stream.next(m.u3);
      stream.next(m.u4);
    }

    ROS_DECLARE_ALLINONE_SERIALIZER
  }; // struct actuator_

} // namespace serialization
} // namespace ros

namespace ros
{
namespace message_operations
{

template<class ContainerAllocator>
struct Printer< ::gazebo_example::actuator_<ContainerAllocator> >
{
  template<typename Stream> static void stream(Stream& s, const std::string& indent, const ::gazebo_example::actuator_<ContainerAllocator>& v)
  {
    s << indent << "u1: ";
    Printer<double>::stream(s, indent + "  ", v.u1);
    s << indent << "u2: ";
    Printer<double>::stream(s, indent + "  ", v.u2);
    s << indent << "u3: ";
    Printer<double>::stream(s, indent + "  ", v.u3);
    s << indent << "u4: ";
    Printer<double>::stream(s, indent + "  ", v.u4);
  }
};

} // namespace message_operations
} // namespace ros

#endif // GAZEBO_EXAMPLE_MESSAGE_ACTUATOR_H
