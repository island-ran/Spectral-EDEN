;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::DtgBagC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'DtgBagC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::DTGBAGC")
  (make-package "EXP_COMM_MSGS::DTGBAGC"))

(in-package "ROS")
;;//! \htmlinclude DtgBagC.msg.html
(if (not (find-package "SWARM_EXP_MSGS"))
  (ros::roseus-add-msgs "swarm_exp_msgs"))


(defclass exp_comm_msgs::DtgBagC
  :super ros::object
  :slots (_pub_t _id _from_uav _FFedges _HFedges _HHedges _Hnodes _Fnodes ))

(defmethod exp_comm_msgs::DtgBagC
  (:init
   (&key
    ((:pub_t __pub_t) 0.0)
    ((:id __id) 0)
    ((:from_uav __from_uav) 0)
    ((:FFedges __FFedges) ())
    ((:HFedges __HFedges) ())
    ((:HHedges __HHedges) ())
    ((:Hnodes __Hnodes) ())
    ((:Fnodes __Fnodes) ())
    )
   (send-super :init)
   (setq _pub_t (float __pub_t))
   (setq _id (round __id))
   (setq _from_uav (round __from_uav))
   (setq _FFedges __FFedges)
   (setq _HFedges __HFedges)
   (setq _HHedges __HHedges)
   (setq _Hnodes __Hnodes)
   (setq _Fnodes __Fnodes)
   self)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:FFedges
   (&rest __FFedges)
   (if (keywordp (car __FFedges))
       (send* _FFedges __FFedges)
     (progn
       (if __FFedges (setq _FFedges (car __FFedges)))
       _FFedges)))
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
    ;; float64 _pub_t
    8
    ;; uint32 _id
    4
    ;; uint8 _from_uav
    1
    ;; swarm_exp_msgs/DtgFFEdge[] _FFedges
    (apply #'+ (send-all _FFedges :serialization-length)) 4
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
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint32 _id
       (write-long _id s)
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; swarm_exp_msgs/DtgFFEdge[] _FFedges
     (write-long (length _FFedges) s)
     (dolist (elem _FFedges)
       (send elem :serialize s)
       )
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
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint32 _id
     (setq _id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; swarm_exp_msgs/DtgFFEdge[] _FFedges
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _FFedges (let (r) (dotimes (i n) (push (instance swarm_exp_msgs::DtgFFEdge :init) r)) r))
     (dolist (elem- _FFedges)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
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

(setf (get exp_comm_msgs::DtgBagC :md5sum-) "ff41106c2b664f3ededcb2a753d90eeb")
(setf (get exp_comm_msgs::DtgBagC :datatype-) "exp_comm_msgs/DtgBagC")
(setf (get exp_comm_msgs::DtgBagC :definition-)
      "float64 pub_t
uint32 id
uint8 from_uav
swarm_exp_msgs/DtgFFEdge[] FFedges
swarm_exp_msgs/DtgHFEdge[] HFedges
swarm_exp_msgs/DtgHHEdge[] HHedges
swarm_exp_msgs/DtgHNode[] Hnodes
swarm_exp_msgs/DtgFNode[] Fnodes 
================================================================================
MSG: swarm_exp_msgs/DtgFFEdge
uint16 head_f_id
uint16 tail_f_id
uint32[] points_idx #low res map

uint8 head_v_id #viewpoint id of head fn
uint8 tail_v_id #viewpoint id of tail fn
uint8 ffe_id    #who creates

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



(provide :exp_comm_msgs/DtgBagC "ff41106c2b664f3ededcb2a753d90eeb")


