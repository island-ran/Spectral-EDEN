;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::IdOdom)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'IdOdom (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::IDODOM")
  (make-package "SWARM_EXP_MSGS::IDODOM"))

(in-package "ROS")
;;//! \htmlinclude IdOdom.msg.html
(if (not (find-package "NAV_MSGS"))
  (ros::roseus-add-msgs "nav_msgs"))


(defclass swarm_exp_msgs::IdOdom
  :super ros::object
  :slots (_id _odoms ))

(defmethod swarm_exp_msgs::IdOdom
  (:init
   (&key
    ((:id __id) 0)
    ((:odoms __odoms) (instance nav_msgs::Odometry :init))
    )
   (send-super :init)
   (setq _id (round __id))
   (setq _odoms __odoms)
   self)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:odoms
   (&rest __odoms)
   (if (keywordp (car __odoms))
       (send* _odoms __odoms)
     (progn
       (if __odoms (setq _odoms (car __odoms)))
       _odoms)))
  (:serialization-length
   ()
   (+
    ;; uint8 _id
    1
    ;; nav_msgs/Odometry _odoms
    (send _odoms :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _id
       (write-byte _id s)
     ;; nav_msgs/Odometry _odoms
       (send _odoms :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _id
     (setq _id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; nav_msgs/Odometry _odoms
     (send _odoms :deserialize buf ptr-) (incf ptr- (send _odoms :serialization-length))
   ;;
   self)
  )

(setf (get swarm_exp_msgs::IdOdom :md5sum-) "21f097ee1c09e8e43ed1e3e67cc0f654")
(setf (get swarm_exp_msgs::IdOdom :datatype-) "swarm_exp_msgs/IdOdom")
(setf (get swarm_exp_msgs::IdOdom :definition-)
      "# immediate for local or group, slow timer for general

uint8 id
nav_msgs/Odometry odoms
================================================================================
MSG: nav_msgs/Odometry
# This represents an estimate of a position and velocity in free space.  
# The pose in this message should be specified in the coordinate frame given by header.frame_id.
# The twist in this message should be specified in the coordinate frame given by the child_frame_id
Header header
string child_frame_id
geometry_msgs/PoseWithCovariance pose
geometry_msgs/TwistWithCovariance twist

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
string frame_id

================================================================================
MSG: geometry_msgs/PoseWithCovariance
# This represents a pose in free space with uncertainty.

Pose pose

# Row-major representation of the 6x6 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[36] covariance

================================================================================
MSG: geometry_msgs/Pose
# A representation of pose in free space, composed of position and orientation. 
Point position
Quaternion orientation

================================================================================
MSG: geometry_msgs/Point
# This contains the position of a point in free space
float64 x
float64 y
float64 z

================================================================================
MSG: geometry_msgs/Quaternion
# This represents an orientation in free space in quaternion form.

float64 x
float64 y
float64 z
float64 w

================================================================================
MSG: geometry_msgs/TwistWithCovariance
# This expresses velocity in free space with uncertainty.

Twist twist

# Row-major representation of the 6x6 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[36] covariance

================================================================================
MSG: geometry_msgs/Twist
# This expresses velocity in free space broken into its linear and angular parts.
Vector3 linear
Vector3 angular

================================================================================
MSG: geometry_msgs/Vector3
# This represents a vector in free space. 
# It is only meant to represent a direction. Therefore, it does not
# make sense to apply a translation to it (e.g., when applying a 
# generic rigid transformation to a Vector3, tf2 will only apply the
# rotation). If you want your data to be translatable too, use the
# geometry_msgs/Point message instead.

float64 x
float64 y
float64 z
")



(provide :swarm_exp_msgs/IdOdom "21f097ee1c09e8e43ed1e3e67cc0f654")


