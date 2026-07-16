;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::IdPoseC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'IdPoseC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::IDPOSEC")
  (make-package "EXP_COMM_MSGS::IDPOSEC"))

(in-package "ROS")
;;//! \htmlinclude IdPoseC.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass exp_comm_msgs::IdPoseC
  :super ros::object
  :slots (_id _pub_t _pose ))

(defmethod exp_comm_msgs::IdPoseC
  (:init
   (&key
    ((:id __id) 0)
    ((:pub_t __pub_t) 0.0)
    ((:pose __pose) (instance geometry_msgs::Pose :init))
    )
   (send-super :init)
   (setq _id (round __id))
   (setq _pub_t (float __pub_t))
   (setq _pose __pose)
   self)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:pose
   (&rest __pose)
   (if (keywordp (car __pose))
       (send* _pose __pose)
     (progn
       (if __pose (setq _pose (car __pose)))
       _pose)))
  (:serialization-length
   ()
   (+
    ;; uint8 _id
    1
    ;; float64 _pub_t
    8
    ;; geometry_msgs/Pose _pose
    (send _pose :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _id
       (write-byte _id s)
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; geometry_msgs/Pose _pose
       (send _pose :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _id
     (setq _id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; geometry_msgs/Pose _pose
     (send _pose :deserialize buf ptr-) (incf ptr- (send _pose :serialization-length))
   ;;
   self)
  )

(setf (get exp_comm_msgs::IdPoseC :md5sum-) "d7bae89dc6e8db6dcfb244807a95d689")
(setf (get exp_comm_msgs::IdPoseC :datatype-) "exp_comm_msgs/IdPoseC")
(setf (get exp_comm_msgs::IdPoseC :definition-)
      "uint8 id
float64 pub_t
geometry_msgs/Pose pose
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

")



(provide :exp_comm_msgs/IdPoseC "d7bae89dc6e8db6dcfb244807a95d689")


