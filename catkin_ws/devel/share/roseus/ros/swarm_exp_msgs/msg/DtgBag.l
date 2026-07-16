;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgBag)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgBag (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGBAG")
  (make-package "SWARM_EXP_MSGS::DTGBAG"))

(in-package "ROS")
;;//! \htmlinclude DtgBag.msg.html


(defclass swarm_exp_msgs::DtgBag
  :super ros::object
  :slots (_to_uavs _id _from_uav _HFedges _HHedges _Hnodes _Fnodes ))

(defmethod swarm_exp_msgs::DtgBag
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:id __id) 0)
    ((:from_uav __from_uav) 0)
    ((:HFedges __HFedges) ())
    ((:HHedges __HHedges) ())
    ((:Hnodes __Hnodes) ())
    ((:Fnodes __Fnodes) ())
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _id (round __id))
   (setq _from_uav (round __from_uav))
   (setq _HFedges __HFedges)
   (setq _HHedges __HHedges)
   (setq _Hnodes __Hnodes)
   (setq _Fnodes __Fnodes)
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:HFedges
   (&rest __HFedges)
   (if (keywordp (car __HFedges))
       (send* _HFedges __HFedges)
     (progn
       (if __HFedges (setq _HFedges (car __HFedges)))
       _HFedges)))
  (:HHedges
   (&rest __HHedges)
   (if (keywordp (car __HHedges))
       (send* _HHedges __HHedges)
     (progn
       (if __HHedges (setq _HHedges (car __HHedges)))
       _HHedges)))
  (:Hnodes
   (&rest __Hnodes)
   (if (keywordp (car __Hnodes))
       (send* _Hnodes __Hnodes)
     (progn
       (if __Hnodes (setq _Hnodes (car __Hnodes)))
       _Hnodes)))
  (:Fnodes
   (&rest __Fnodes)
   (if (keywordp (car __Fnodes))
       (send* _Fnodes __Fnodes)
     (progn
       (if __Fnodes (setq _Fnodes (car __Fnodes)))
       _Fnodes)))
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint32 _id
    4
    ;; uint8 _from_uav
    1
    ;; swarm_exp_msgs/DtgHFEdge[] _HFedges
    (apply #'+ (send-all _HFedges :serialization-length)) 4
    ;; swarm_exp_msgs/DtgHHEdge[] _HHedges
    (apply #'+ (send-all _HHedges :serialization-length)) 4
    ;; swarm_exp_msgs/DtgHNode[] _Hnodes
    (apply #'+ (send-all _Hnodes :serialization-length)) 4
    ;; swarm_exp_msgs/DtgFNode[] _Fnodes
    (apply #'+ (send-all _Fnodes :serialization-length)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8[] _to_uavs
     (write-long (length _to_uavs) s)
     (princ _to_uavs s)
     ;; uint32 _id
       (write-long _id s)
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; swarm_exp_msgs/DtgHFEdge[] _HFedges
     (write-long (length _HFedges) s)
     (dolist (elem _HFedges)
       (send elem :serialize s)
       )
     ;; swarm_exp_msgs/DtgHHEdge[] _HHedges
     (write-long (length _HHedges) s)
     (dolist (elem _HHedges)
       (send elem :serialize s)
       )
     ;; swarm_exp_msgs/DtgHNode[] _Hnodes
     (write-long (length _Hnodes) s)
     (dolist (elem _Hnodes)
       (send elem :serialize s)
       )
     ;; swarm_exp_msgs/DtgFNode[] _Fnodes
     (write-long (length _Fnodes) s)
     (dolist (elem _Fnodes)
       (send elem :serialize s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8[] _to_uavs
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _to_uavs (make-array n :element-type :char))
     (replace _to_uavs buf :start2 ptr-) (incf ptr- n))
   ;; uint32 _id
     (setq _id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; swarm_exp_msgs/DtgHFEdge[] _HFedges
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _HFedges (let (r) (dotimes (i n) (push (instance swarm_exp_msgs::DtgHFEdge :init) r)) r))
     (dolist (elem- _HFedges)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; swarm_exp_msgs/DtgHHEdge[] _HHedges
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _HHedges (let (r) (dotimes (i n) (push (instance swarm_exp_msgs::DtgHHEdge :init) r)) r))
     (dolist (elem- _HHedges)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; swarm_exp_msgs/DtgHNode[] _Hnodes
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _Hnodes (let (r) (dotimes (i n) (push (instance swarm_exp_msgs::DtgHNode :init) r)) r))
     (dolist (elem- _Hnodes)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; swarm_exp_msgs/DtgFNode[] _Fnodes
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _Fnodes (let (r) (dotimes (i n) (push (instance swarm_exp_msgs::DtgFNode :init) r)) r))
     (dolist (elem- _Fnodes)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgBag :md5sum-) "0fad25fdf73ad19fd3ef0fed38c94965")
(setf (get swarm_exp_msgs::DtgBag :datatype-) "swarm_exp_msgs/DtgBag")
(setf (get swarm_exp_msgs::DtgBag :definition-)
      "# groupe broad cast (quick timer for local or group / slow timer for general)

uint8[] to_uavs
uint32 id
uint8 from_uav

swarm_exp_msgs/DtgHFEdge[] HFedges
swarm_exp_msgs/DtgHHEdge[] HHedges
swarm_exp_msgs/DtgHNode[] Hnodes
swarm_exp_msgs/DtgFNode[] Fnodes 
================================================================================
MSG: swarm_exp_msgs/DtgHFEdge
uint32[] points_idx #low res map
float64 pub_t
uint16 f_id
uint32 h_id
uint8 vp_id
#uint8 hfe_id        #who creates

================================================================================
MSG: swarm_exp_msgs/DtgHHEdge
#uint8 hhe_id        #who creates
#bool erase 
float64 pub_t
uint32[] points_idx #low res map
uint32 head_h_id
uint32 tail_h_id
================================================================================
MSG: swarm_exp_msgs/DtgHNode
uint32 h_id
uint32 pos_idx #low res map
================================================================================
MSG: swarm_exp_msgs/DtgFNode
uint16 f_id
bool alive
uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)
uint8 need_help # ask other uav for edge

")



(provide :swarm_exp_msgs/DtgBag "0fad25fdf73ad19fd3ef0fed38c94965")


