;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupMap)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupMap (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPMAP")
  (make-package "SWARM_EXP_MSGS::GROUPMAP"))

(in-package "ROS")
;;//! \htmlinclude GroupMap.msg.html


(defclass swarm_exp_msgs::GroupMap
  :super ros::object
  :slots ())

(defmethod swarm_exp_msgs::GroupMap
  (:init
   (&key
    )
   (send-super :init)
   self)
  (:serialization-length
   ()
   (+
    0
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupMap :md5sum-) "d41d8cd98f00b204e9800998ecf8427e")
(setf (get swarm_exp_msgs::GroupMap :datatype-) "swarm_exp_msgs/GroupMap")
(setf (get swarm_exp_msgs::GroupMap :definition-)
      "# inside group broad cast (quick timer)

")



(provide :swarm_exp_msgs/GroupMap "d41d8cd98f00b204e9800998ecf8427e")


