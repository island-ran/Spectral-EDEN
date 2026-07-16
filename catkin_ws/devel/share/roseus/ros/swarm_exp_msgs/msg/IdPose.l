;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::IdPose)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'IdPose (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::IDPOSE")
  (make-package "SWARM_EXP_MSGS::IDPOSE"))

(in-package "ROS")
;;//! \htmlinclude IdPose.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass swarm_exp_msgs::IdPose
  :super ros::object
  :slots (_to_uavs _id _pub_t _pose ))

(defmethod swarm_exp_msgs::IdPose
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:id __id) 0)
    ((:pub_t __pub_t) 0.0)
    ((:pose __pose) (instance geometry_msgs::Pose :init))
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _id (round __id))
   (setq _pub_t (float __pub_t))
   (setq _pose __pose)
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
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
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
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
     ;; uint8[] _to_uavs
     (write-long (length _to_uavs) s)
     (princ _to_uavs s)
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
   ;; uint8[] _to_uavs
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _to_uavs (make-array n :element-type :char))
     (replace _to_uavs buf :start2 ptr-) (incf ptr- n))
   ;; uint8 _id
     (setq _id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; geometry_msgs/Pose _pose
     (send _pose :deserialize buf ptr-) (incf ptr- (send _pose :serialization-length))
   ;;
   self)
  )

(setf (get swarm_exp_msgs::IdPose :md5sum-) "cc5f67458fe878e0f625ae446543e38e")
(setf (get swarm_exp_msgs::IdPose :datatype-) "swarm_exp_msgs/IdPose")
(setf (get swarm_exp_msgs::IdPose :definition-)
      "# immediate for local or group, slow timer for general
uint8[] to_uavs

uint8 id
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



(provide :swarm_exp_msgs/IdPose "cc5f67458fe878e0f625ae446543e38e")


