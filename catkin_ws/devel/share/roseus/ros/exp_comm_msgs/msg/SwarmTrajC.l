;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::SwarmTrajC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'SwarmTrajC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::SWARMTRAJC")
  (make-package "EXP_COMM_MSGS::SWARMTRAJC"))

(in-package "ROS")
;;//! \htmlinclude SwarmTrajC.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))


(defclass exp_comm_msgs::SwarmTrajC
  :super ros::object
  :slots (_pub_t _id _start_t _order_p _t_p _coef_p ))

(defmethod exp_comm_msgs::SwarmTrajC
  (:init
   (&key
    ((:pub_t __pub_t) 0.0)
    ((:id __id) 0)
    ((:start_t __start_t) 0.0)
    ((:order_p __order_p) 0)
    ((:t_p __t_p) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:coef_p __coef_p) ())
    )
   (send-super :init)
   (setq _pub_t (float __pub_t))
   (setq _id (round __id))
   (setq _start_t (float __start_t))
   (setq _order_p (round __order_p))
   (setq _t_p __t_p)
   (setq _coef_p __coef_p)
   self)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
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
    ;; float64 _pub_t
    8
    ;; uint8 _id
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
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _id
       (write-byte _id s)
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
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _id
     (setq _id (sys::peek buf ptr- :char)) (incf ptr- 1)
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

(setf (get exp_comm_msgs::SwarmTrajC :md5sum-) "b412e83c14c239c5f9f2ca748f0bead0")
(setf (get exp_comm_msgs::SwarmTrajC :datatype-) "exp_comm_msgs/SwarmTrajC")
(setf (get exp_comm_msgs::SwarmTrajC :definition-)
      "float64 pub_t

uint8 id

float64 start_t
int8 order_p
float32[] t_p
geometry_msgs/Point[] coef_p

# int8 order_yaw
# float32[] t_yaw
# float32[] coef_yaw
================================================================================
MSG: geometry_msgs/Point
# This contains the position of a point in free space
float64 x
float64 y
float64 z

")



(provide :exp_comm_msgs/SwarmTrajC "b412e83c14c239c5f9f2ca748f0bead0")


