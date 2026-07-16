;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupHelperReq)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupHelperReq (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPHELPERREQ")
  (make-package "SWARM_EXP_MSGS::GROUPHELPERREQ"))

(in-package "ROS")
;;//! \htmlinclude GroupHelperReq.msg.html


(defclass swarm_exp_msgs::GroupHelperReq
  :super ros::object
  :slots (_to_uavs _from_uav _helper_ids _cost ))

(defmethod swarm_exp_msgs::GroupHelperReq
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:from_uav __from_uav) 0)
    ((:helper_ids __helper_ids) (make-array 0 :initial-element 0 :element-type :char))
    ((:cost __cost) 0.0)
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _from_uav (round __from_uav))
   (setq _helper_ids __helper_ids)
   (setq _cost (float __cost))
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:helper_ids
   (&optional __helper_ids)
   (if __helper_ids (setq _helper_ids __helper_ids)) _helper_ids)
  (:cost
   (&optional __cost)
   (if __cost (setq _cost __cost)) _cost)
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint8 _from_uav
    1
    ;; uint8[] _helper_ids
    (* 1    (length _helper_ids)) 4
    ;; float32 _cost
    4
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
     ;; uint8[] _helper_ids
     (write-long (length _helper_ids) s)
     (princ _helper_ids s)
     ;; float32 _cost
       (sys::poke _cost (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
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
   ;; uint8[] _helper_ids
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _helper_ids (make-array n :element-type :char))
     (replace _helper_ids buf :start2 ptr-) (incf ptr- n))
   ;; float32 _cost
     (setq _cost (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupHelperReq :md5sum-) "de1dd505e44758dd2e830a3b48f4bffe")
(setf (get swarm_exp_msgs::GroupHelperReq :datatype-) "swarm_exp_msgs/GroupHelperReq")
(setf (get swarm_exp_msgs::GroupHelperReq :definition-)
      "# to certain group trooper (immediate answer)
uint8[] to_uavs
uint8 from_uav

uint8[] helper_ids
float32 cost
")



(provide :swarm_exp_msgs/GroupHelperReq "de1dd505e44758dd2e830a3b48f4bffe")


