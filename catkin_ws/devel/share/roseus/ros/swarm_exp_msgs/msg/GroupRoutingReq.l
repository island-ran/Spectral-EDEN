;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupRoutingReq)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupRoutingReq (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPROUTINGREQ")
  (make-package "SWARM_EXP_MSGS::GROUPROUTINGREQ"))

(in-package "ROS")
;;//! \htmlinclude GroupRoutingReq.msg.html


(defclass swarm_exp_msgs::GroupRoutingReq
  :super ros::object
  :slots ())

(defmethod swarm_exp_msgs::GroupRoutingReq
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

(setf (get swarm_exp_msgs::GroupRoutingReq :md5sum-) "d41d8cd98f00b204e9800998ecf8427e")
(setf (get swarm_exp_msgs::GroupRoutingReq :datatype-) "swarm_exp_msgs/GroupRoutingReq")
(setf (get swarm_exp_msgs::GroupRoutingReq :definition-)
      "# inside group, from trooper (Event Timer)

# problem formation and answer space
")



(provide :swarm_exp_msgs/GroupRoutingReq "d41d8cd98f00b204e9800998ecf8427e")


