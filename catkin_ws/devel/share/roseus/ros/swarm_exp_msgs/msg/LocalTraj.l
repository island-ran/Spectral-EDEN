;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::LocalTraj)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'LocalTraj (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::LOCALTRAJ")
  (make-package "SWARM_EXP_MSGS::LOCALTRAJ"))

(in-package "ROS")
;;//! \htmlinclude LocalTraj.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass swarm_exp_msgs::LocalTraj
  :super ros::object
  :slots (_from_uav _to_uavs _state _recover_pt _start_t _order_p _t_p _coef_p _order_yaw _t_yaw _coef_yaw ))

(defmethod swarm_exp_msgs::LocalTraj
  (:init
   (&key
    ((:from_uav __from_uav) 0)
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:state __state) 0)
    ((:recover_pt __recover_pt) (instance geometry_msgs::Point :init))
    ((:start_t __start_t) 0.0)
    ((:order_p __order_p) 0)
    ((:t_p __t_p) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:coef_p __coef_p) ())
    ((:order_yaw __order_yaw) 0)
    ((:t_yaw __t_yaw) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:coef_yaw __coef_yaw) (make-array 0 :initial-element 0.0 :element-type :float))
    )
   (send-super :init)
   (setq _from_uav (round __from_uav))
   (setq _to_uavs __to_uavs)
   (setq _state (round __state))
   (setq _recover_pt __recover_pt)
   (setq _start_t (float __start_t))
   (setq _order_p (round __order_p))
   (setq _t_p __t_p)
   (setq _coef_p __coef_p)
   (setq _order_yaw (round __order_yaw))
   (setq _t_yaw __t_yaw)
   (setq _coef_yaw __coef_yaw)
   self)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:state
   (&optional __state)
   (if __state (setq _state __state)) _state)
  (:recover_pt
   (&rest __recover_pt)
   (if (keywordp (car __recover_pt))
       (send* _recover_pt __recover_pt)
     (progn
       (if __recover_pt (setq _recover_pt (car __recover_pt)))
       _recover_pt)))
  (:start_t
   (&optional __start_t)
   (if __start_t (setq _start_t __start_t)) _start_t)
  (:order_p
   (&optional __order_p)
   (if __order_p (setq _order_p __order_p)) _order_p)
  (:t_p
   (&optional __t_p)
   (if __t_p (setq _t_p __t_p)) _t_p)
  (:coef_p
   (&rest __coef_p)
   (if (keywordp (car __coef_p))
       (send* _coef_p __coef_p)
     (progn
       (if __coef_p (setq _coef_p (car __coef_p)))
       _coef_p)))
  (:order_yaw
   (&optional __order_yaw)
   (if __order_yaw (setq _order_yaw __order_yaw)) _order_yaw)
  (:t_yaw
   (&optional __t_yaw)
   (if __t_yaw (setq _t_yaw __t_yaw)) _t_yaw)
  (:coef_yaw
   (&optional __coef_yaw)
   (if __coef_yaw (setq _coef_yaw __coef_yaw)) _coef_yaw)
  (:serialization-length
   ()
   (+
    ;; uint8 _from_uav
    1
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; int8 _state
    1
    ;; geometry_msgs/Point _recover_pt
    (send _recover_pt :serialization-length)
    ;; float64 _start_t
    8
    ;; int8 _order_p
    1
    ;; float32[] _t_p
    (* 4    (length _t_p)) 4
    ;; geometry_msgs/Point[] _coef_p
    (apply #'+ (send-all _coef_p :serialization-length)) 4
    ;; int8 _order_yaw
    1
    ;; float32[] _t_yaw
    (* 4    (length _t_yaw)) 4
    ;; float32[] _coef_yaw
    (* 4    (length _coef_yaw)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; uint8[] _to_uavs
     (write-long (length _to_uavs) s)
     (princ _to_uavs s)
     ;; int8 _state
       (write-byte _state s)
     ;; geometry_msgs/Point _recover_pt
       (send _recover_pt :serialize s)
     ;; float64 _start_t
       (sys::poke _start_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; int8 _order_p
       (write-byte _order_p s)
     ;; float32[] _t_p
     (write-long (length _t_p) s)
     (dotimes (i (length _t_p))
       (sys::poke (elt _t_p i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; geometry_msgs/Point[] _coef_p
     (write-long (length _coef_p) s)
     (dolist (elem _coef_p)
       (send elem :serialize s)
       )
     ;; int8 _order_yaw
       (write-byte _order_yaw s)
     ;; float32[] _t_yaw
     (write-long (length _t_yaw) s)
     (dotimes (i (length _t_yaw))
       (sys::poke (elt _t_yaw i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; float32[] _coef_yaw
     (write-long (length _coef_yaw) s)
     (dotimes (i (length _coef_yaw))
       (sys::poke (elt _coef_yaw i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8[] _to_uavs
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _to_uavs (make-array n :element-type :char))
     (replace _to_uavs buf :start2 ptr-) (incf ptr- n))
   ;; int8 _state
     (setq _state (sys::peek buf ptr- :char)) (incf ptr- 1)
     (if (> _state 127) (setq _state (- _state 256)))
   ;; geometry_msgs/Point _recover_pt
     (send _recover_pt :deserialize buf ptr-) (incf ptr- (send _recover_pt :serialization-length))
   ;; float64 _start_t
     (setq _start_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; int8 _order_p
     (setq _order_p (sys::peek buf ptr- :char)) (incf ptr- 1)
     (if (> _order_p 127) (setq _order_p (- _order_p 256)))
   ;; float32[] _t_p
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _t_p (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _t_p i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; geometry_msgs/Point[] _coef_p
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _coef_p (let (r) (dotimes (i n) (push (instance geometry_msgs::Point :init) r)) r))
     (dolist (elem- _coef_p)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; int8 _order_yaw
     (setq _order_yaw (sys::peek buf ptr- :char)) (incf ptr- 1)
     (if (> _order_yaw 127) (setq _order_yaw (- _order_yaw 256)))
   ;; float32[] _t_yaw
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _t_yaw (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _t_yaw i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; float32[] _coef_yaw
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _coef_yaw (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _coef_yaw i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;;
   self)
  )

(setf (get swarm_exp_msgs::LocalTraj :md5sum-) "0b2ad7fbcf01f3397172ee31bee3cc0b")
(setf (get swarm_exp_msgs::LocalTraj :datatype-) "swarm_exp_msgs/LocalTraj")
(setf (get swarm_exp_msgs::LocalTraj :definition-)
      "uint8 from_uav
uint8[] to_uavs

int8 state

geometry_msgs/Point recover_pt

float64 start_t
int8 order_p
float32[] t_p
geometry_msgs/Point[] coef_p

int8 order_yaw
float32[] t_yaw
float32[] coef_yaw

================================================================================
MSG: geometry_msgs/Point
# This contains the position of a point in free space
float64 x
float64 y
float64 z

")



(provide :swarm_exp_msgs/LocalTraj "0b2ad7fbcf01f3397172ee31bee3cc0b")


