;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupMemberState)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupMemberState (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPMEMBERSTATE")
  (make-package "SWARM_EXP_MSGS::GROUPMEMBERSTATE"))

(in-package "ROS")
;;//! \htmlinclude GroupMemberState.msg.html


(defclass swarm_exp_msgs::GroupMemberState
  :super ros::object
  :slots (_to_uavs _from_uav _cur_t _targets _work_type ))

(defmethod swarm_exp_msgs::GroupMemberState
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:from_uav __from_uav) 0)
    ((:cur_t __cur_t) 0.0)
    ((:targets __targets) (make-array 0 :initial-element 0 :element-type :integer))
    ((:work_type __work_type) 0)
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _from_uav (round __from_uav))
   (setq _cur_t (float __cur_t))
   (setq _targets __targets)
   (setq _work_type (round __work_type))
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:cur_t
   (&optional __cur_t)
   (if __cur_t (setq _cur_t __cur_t)) _cur_t)
  (:targets
   (&optional __targets)
   (if __targets (setq _targets __targets)) _targets)
  (:work_type
   (&optional __work_type)
   (if __work_type (setq _work_type __work_type)) _work_type)
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint8 _from_uav
    1
    ;; float64 _cur_t
    8
    ;; uint32[] _targets
    (* 4    (length _targets)) 4
    ;; uint8 _work_type
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
     ;; float64 _cur_t
       (sys::poke _cur_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint32[] _targets
     (write-long (length _targets) s)
     (dotimes (i (length _targets))
       (write-long (elt _targets i) s)
       )
     ;; uint8 _work_type
       (write-byte _work_type s)
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
   ;; float64 _cur_t
     (setq _cur_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint32[] _targets
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _targets (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _targets i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint8 _work_type
     (setq _work_type (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupMemberState :md5sum-) "abb6338d4d1acda965b2e312dd0c2bb4")
(setf (get swarm_exp_msgs::GroupMemberState :datatype-) "swarm_exp_msgs/GroupMemberState")
(setf (get swarm_exp_msgs::GroupMemberState :definition-)
      "# group broad cast (quick timer)
uint8[] to_uavs
uint8 from_uav

float64 cur_t
uint32[] targets
uint8 work_type #cover, explore, follow, wait
")



(provide :swarm_exp_msgs/GroupMemberState "abb6338d4d1acda965b2e312dd0c2bb4")


