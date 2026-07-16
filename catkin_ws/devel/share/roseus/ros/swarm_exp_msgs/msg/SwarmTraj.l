;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::SwarmTraj)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'SwarmTraj (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::SWARMTRAJ")
  (make-package "SWARM_EXP_MSGS::SWARMTRAJ"))

(in-package "ROS")
;;//! \htmlinclude SwarmTraj.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass swarm_exp_msgs::SwarmTraj
  :super ros::object
  :slots (_to_uavs _from_uav _start_t _order_p _t_p _coef_p ))

(defmethod swarm_exp_msgs::SwarmTraj
  (:init
   (&key
    ((:to_uavs __to_uavs) (make-array 0 :initial-element 0 :element-type :char))
    ((:from_uav __from_uav) 0)
    ((:start_t __start_t) 0.0)
    ((:order_p __order_p) 0)
    ((:t_p __t_p) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:coef_p __coef_p) ())
    )
   (send-super :init)
   (setq _to_uavs __to_uavs)
   (setq _from_uav (round __from_uav))
   (setq _start_t (float __start_t))
   (setq _order_p (round __order_p))
   (setq _t_p __t_p)
   (setq _coef_p __coef_p)
   self)
  (:to_uavs
   (&optional __to_uavs)
   (if __to_uavs (setq _to_uavs __to_uavs)) _to_uavs)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
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
  (:serialization-length
   ()
   (+
    ;; uint8[] _to_uavs
    (* 1    (length _to_uavs)) 4
    ;; uint8 _from_uav
    1
    ;; float64 _start_t
    8
    ;; int8 _order_p
    1
    ;; float32[] _t_p
    (* 4    (length _t_p)) 4
    ;; geometry_msgs/Point[] _coef_p
    (apply #'+ (send-all _coef_p :serialization-length)) 4
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
   ;;
   self)
  )

(setf (get swarm_exp_msgs::SwarmTraj :md5sum-) "ad6c481b3a1cd3eae78b78f76fc6821d")
(setf (get swarm_exp_msgs::SwarmTraj :datatype-) "swarm_exp_msgs/SwarmTraj")
(setf (get swarm_exp_msgs::SwarmTraj :definition-)
      "# immediate for local or group

uint8[] to_uavs
uint8 from_uav

float64 start_t
int8 order_p
float32[] t_p
geometry_msgs/Point[] coef_p


================================================================================
MSG: geometry_msgs/Point
# This contains the position of a point in free space
float64 x
float64 y
float64 z

")



(provide :swarm_exp_msgs/SwarmTraj "ad6c481b3a1cd3eae78b78f76fc6821d")


