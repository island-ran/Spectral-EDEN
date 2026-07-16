;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupState)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupState (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPSTATE")
  (make-package "SWARM_EXP_MSGS::GROUPSTATE"))

(in-package "ROS")
;;//! \htmlinclude GroupState.msg.html


(defclass swarm_exp_msgs::GroupState
  :super ros::object
  :slots (_to_uavs _from_uav _cur_t _group_size _trooper_targets _potential _coverage_work_load _state ))

(defmethod swarm_exp_msgs::GroupState
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:from_uav __from_uav) 0)
    ((:cur_t __cur_t) 0.0)
    ((:group_size __group_size) 0)
    ((:trooper_targets __trooper_targets) (make-array 0 :initial-element 0 :element-type :integer))
    ((:potential __potential) 0)
    ((:coverage_work_load __coverage_work_load) 0)
    ((:state __state) 0)
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _from_uav (round __from_uav))
   (setq _cur_t (float __cur_t))
   (setq _group_size (round __group_size))
   (setq _trooper_targets __trooper_targets)
   (setq _potential (round __potential))
   (setq _coverage_work_load (round __coverage_work_load))
   (setq _state (round __state))
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
  (:group_size
   (&optional __group_size)
   (if __group_size (setq _group_size __group_size)) _group_size)
  (:trooper_targets
   (&optional __trooper_targets)
   (if __trooper_targets (setq _trooper_targets __trooper_targets)) _trooper_targets)
  (:potential
   (&optional __potential)
   (if __potential (setq _potential __potential)) _potential)
  (:coverage_work_load
   (&optional __coverage_work_load)
   (if __coverage_work_load (setq _coverage_work_load __coverage_work_load)) _coverage_work_load)
  (:state
   (&optional __state)
   (if __state (setq _state __state)) _state)
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint8 _from_uav
    1
    ;; float64 _cur_t
    8
    ;; uint8 _group_size
    1
    ;; uint32[] _trooper_targets
    (* 4    (length _trooper_targets)) 4
    ;; uint16 _potential
    2
    ;; uint16 _coverage_work_load
    2
    ;; uint8 _state
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
     ;; uint8 _group_size
       (write-byte _group_size s)
     ;; uint32[] _trooper_targets
     (write-long (length _trooper_targets) s)
     (dotimes (i (length _trooper_targets))
       (write-long (elt _trooper_targets i) s)
       )
     ;; uint16 _potential
       (write-word _potential s)
     ;; uint16 _coverage_work_load
       (write-word _coverage_work_load s)
     ;; uint8 _state
       (write-byte _state s)
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
   ;; uint8 _group_size
     (setq _group_size (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint32[] _trooper_targets
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _trooper_targets (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _trooper_targets i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint16 _potential
     (setq _potential (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _coverage_work_load
     (setq _coverage_work_load (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint8 _state
     (setq _state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupState :md5sum-) "17f63ea80422701593a5c33c707b7615")
(setf (get swarm_exp_msgs::GroupState :datatype-) "swarm_exp_msgs/GroupState")
(setf (get swarm_exp_msgs::GroupState :definition-)
      "# inter group broad cast (slow timer)
uint8[] to_uavs
uint8 from_uav

float64 cur_t
uint8 group_size
uint32[] trooper_targets
uint16 potential #calculated unknown space number, for helpers
uint16 coverage_work_load
uint8 state # 0000 00(need help)(exp/cover)


")



(provide :swarm_exp_msgs/GroupState "17f63ea80422701593a5c33c707b7615")


