;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgHNode)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgHNode (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGHNODE")
  (make-package "SWARM_EXP_MSGS::DTGHNODE"))

(in-package "ROS")
;;//! \htmlinclude DtgHNode.msg.html


(defclass swarm_exp_msgs::DtgHNode
  :super ros::object
  :slots (_h_id _pos_idx ))

(defmethod swarm_exp_msgs::DtgHNode
  (:init
   (&key
    ((:h_id __h_id) 0)
    ((:pos_idx __pos_idx) 0)
    )
   (send-super :init)
   (setq _h_id (round __h_id))
   (setq _pos_idx (round __pos_idx))
   self)
  (:h_id
   (&optional __h_id)
   (if __h_id (setq _h_id __h_id)) _h_id)
  (:pos_idx
   (&optional __pos_idx)
   (if __pos_idx (setq _pos_idx __pos_idx)) _pos_idx)
  (:serialization-length
   ()
   (+
    ;; uint32 _h_id
    4
    ;; uint32 _pos_idx
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32 _h_id
       (write-long _h_id s)
     ;; uint32 _pos_idx
       (write-long _pos_idx s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32 _h_id
     (setq _h_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _pos_idx
     (setq _pos_idx (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgHNode :md5sum-) "61b258874ecfcb669e1516b9e9ebe709")
(setf (get swarm_exp_msgs::DtgHNode :datatype-) "swarm_exp_msgs/DtgHNode")
(setf (get swarm_exp_msgs::DtgHNode :definition-)
      "uint32 h_id
uint32 pos_idx #low res map
")



(provide :swarm_exp_msgs/DtgHNode "61b258874ecfcb669e1516b9e9ebe709")


