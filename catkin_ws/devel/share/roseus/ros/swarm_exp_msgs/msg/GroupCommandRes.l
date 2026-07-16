;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupCommandRes)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupCommandRes (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPCOMMANDRES")
  (make-package "SWARM_EXP_MSGS::GROUPCOMMANDRES"))

(in-package "ROS")
;;//! \htmlinclude GroupCommandRes.msg.html


(defclass swarm_exp_msgs::GroupCommandRes
  :super ros::object
  :slots ())

(defmethod swarm_exp_msgs::GroupCommandRes
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

(setf (get swarm_exp_msgs::GroupCommandRes :md5sum-) "d41d8cd98f00b204e9800998ecf8427e")
(setf (get swarm_exp_msgs::GroupCommandRes :datatype-) "swarm_exp_msgs/GroupCommandRes")
(setf (get swarm_exp_msgs::GroupCommandRes :definition-)
      "# inside group, to trooper (Immediate answer)



")



(provide :swarm_exp_msgs/GroupCommandRes "d41d8cd98f00b204e9800998ecf8427e")


