;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupRoutingRes)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupRoutingRes (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPROUTINGRES")
  (make-package "SWARM_EXP_MSGS::GROUPROUTINGRES"))

(in-package "ROS")
;;//! \htmlinclude GroupRoutingRes.msg.html


(defclass swarm_exp_msgs::GroupRoutingRes
  :super ros::object
  :slots ())

(defmethod swarm_exp_msgs::GroupRoutingRes
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

(setf (get swarm_exp_msgs::GroupRoutingRes :md5sum-) "d41d8cd98f00b204e9800998ecf8427e")
(setf (get swarm_exp_msgs::GroupRoutingRes :datatype-) "swarm_exp_msgs/GroupRoutingRes")
(setf (get swarm_exp_msgs::GroupRoutingRes :definition-)
      "# inside group, to trooper (Immediate answer)

# best answer
")



(provide :swarm_exp_msgs/GroupRoutingRes "d41d8cd98f00b204e9800998ecf8427e")


