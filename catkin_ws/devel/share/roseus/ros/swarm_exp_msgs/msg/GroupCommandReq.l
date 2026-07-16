;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::GroupCommandReq)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'GroupCommandReq (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::GROUPCOMMANDREQ")
  (make-package "SWARM_EXP_MSGS::GROUPCOMMANDREQ"))

(in-package "ROS")
;;//! \htmlinclude GroupCommandReq.msg.html


(defclass swarm_exp_msgs::GroupCommandReq
  :super ros::object
  :slots (_to_uavs _cur_t _dispatch_type _departure_EROIs _departure_status _helped_group_id ))

(defmethod swarm_exp_msgs::GroupCommandReq
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:cur_t __cur_t) 0.0)
    ((:dispatch_type __dispatch_type) 0)
    ((:departure_EROIs __departure_EROIs) (make-array 0 :initial-element 0 :element-type :integer))
    ((:departure_status __departure_status) 0)
    ((:helped_group_id __helped_group_id) 0)
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _cur_t (float __cur_t))
   (setq _dispatch_type (round __dispatch_type))
   (setq _departure_EROIs __departure_EROIs)
   (setq _departure_status (round __departure_status))
   (setq _helped_group_id (round __helped_group_id))
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:cur_t
   (&optional __cur_t)
   (if __cur_t (setq _cur_t __cur_t)) _cur_t)
  (:dispatch_type
   (&optional __dispatch_type)
   (if __dispatch_type (setq _dispatch_type __dispatch_type)) _dispatch_type)
  (:departure_EROIs
   (&optional __departure_EROIs)
   (if __departure_EROIs (setq _departure_EROIs __departure_EROIs)) _departure_EROIs)
  (:departure_status
   (&optional __departure_status)
   (if __departure_status (setq _departure_status __departure_status)) _departure_status)
  (:helped_group_id
   (&optional __helped_group_id)
   (if __helped_group_id (setq _helped_group_id __helped_group_id)) _helped_group_id)
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; float64 _cur_t
    8
    ;; uint8 _dispatch_type
    1
    ;; uint32[] _departure_EROIs
    (* 4    (length _departure_EROIs)) 4
    ;; uint8 _departure_status
    1
    ;; uint8 _helped_group_id
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8[] _to_uavs
     (write-long (length _to_uavs) s)
     (princ _to_uavs s)
     ;; float64 _cur_t
       (sys::poke _cur_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _dispatch_type
       (write-byte _dispatch_type s)
     ;; uint32[] _departure_EROIs
     (write-long (length _departure_EROIs) s)
     (dotimes (i (length _departure_EROIs))
       (write-long (elt _departure_EROIs i) s)
       )
     ;; uint8 _departure_status
       (write-byte _departure_status s)
     ;; uint8 _helped_group_id
       (write-byte _helped_group_id s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8[] _to_uavs
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _to_uavs (make-array n :element-type :char))
     (replace _to_uavs buf :start2 ptr-) (incf ptr- n))
   ;; float64 _cur_t
     (setq _cur_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _dispatch_type
     (setq _dispatch_type (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint32[] _departure_EROIs
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _departure_EROIs (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _departure_EROIs i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint8 _departure_status
     (setq _departure_status (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _helped_group_id
     (setq _helped_group_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::GroupCommandReq :md5sum-) "8907e16fca4b37e09a5750aaba75e3cc")
(setf (get swarm_exp_msgs::GroupCommandReq :datatype-) "swarm_exp_msgs/GroupCommandReq")
(setf (get swarm_exp_msgs::GroupCommandReq :definition-)
      "# inside group, from trooper (Quick timmer)
uint8[] to_uavs
# uint8 from_uav

float64 cur_t
# 1: Departure; 2: Help, 3: switch trooper
uint8 dispatch_type 

# departure
uint32[] departure_EROIs
uint8 departure_status #0: trooper; 1: infantry

#help
uint8 helped_group_id



")



(provide :swarm_exp_msgs/GroupCommandReq "8907e16fca4b37e09a5750aaba75e3cc")


