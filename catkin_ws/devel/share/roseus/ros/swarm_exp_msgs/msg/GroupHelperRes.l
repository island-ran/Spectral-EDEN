;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupHelperRes)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupHelperRes (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPHELPERRES")
  (make-package "SWARM_EXP_MSGS::GROUPHELPERRES"))

(in-package "ROS")
;;//! \htmlinclude GroupHelperRes.msg.html


(defclass swarm_exp_msgs::GroupHelperRes
  :super ros::object
  :slots (_to_uavs _from_uav _allowed_num ))

(defmethod swarm_exp_msgs::GroupHelperRes
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:from_uav __from_uav) 0)
    ((:allowed_num __allowed_num) 0)
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _from_uav (round __from_uav))
   (setq _allowed_num (round __allowed_num))
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:allowed_num
   (&optional __allowed_num)
   (if __allowed_num (setq _allowed_num __allowed_num)) _allowed_num)
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint8 _from_uav
    1
    ;; uint8 _allowed_num
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8[] _to_uavs
     (write-long (length _to_uavs) s)
     (princ _to_uavs s)
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; uint8 _allowed_num
       (write-byte _allowed_num s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8[] _to_uavs
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _to_uavs (make-array n :element-type :char))
     (replace _to_uavs buf :start2 ptr-) (incf ptr- n))
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _allowed_num
     (setq _allowed_num (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupHelperRes :md5sum-) "f1d5958dfe9e4c0dcd170ba5dda86e59")
(setf (get swarm_exp_msgs::GroupHelperRes :datatype-) "swarm_exp_msgs/GroupHelperRes")
(setf (get swarm_exp_msgs::GroupHelperRes :definition-)
      "# to certain group trooper (immediate answer)
uint8[] to_uavs
uint8 from_uav

uint8 allowed_num
")



(provide :swarm_exp_msgs/GroupHelperRes "f1d5958dfe9e4c0dcd170ba5dda86e59")


