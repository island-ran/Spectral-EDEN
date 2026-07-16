;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::SwarmStateC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'SwarmStateC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::SWARMSTATEC")
  (make-package "EXP_COMM_MSGS::SWARMSTATEC"))

(in-package "ROS")
;;//! \htmlinclude SwarmStateC.msg.html


(defclass exp_comm_msgs::SwarmStateC
  :super ros::object
  :slots (_pub_t _from_uav _flags _local_fn _dist_to_local_fn _connect_fn _dist_to_connect_fn _connect_hn _hn_pos_idx _dist_to_connect_hn ))

(defmethod exp_comm_msgs::SwarmStateC
  (:init
   (&key
    ((:pub_t __pub_t) 0.0)
    ((:from_uav __from_uav) 0)
    ((:flags __flags) 0)
    ((:local_fn __local_fn) (make-array 0 :initial-element 0 :element-type :integer))
    ((:dist_to_local_fn __dist_to_local_fn) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:connect_fn __connect_fn) (make-array 0 :initial-element 0 :element-type :integer))
    ((:dist_to_connect_fn __dist_to_connect_fn) (make-array 0 :initial-element 0.0 :element-type :float))
    ((:connect_hn __connect_hn) (make-array 0 :initial-element 0 :element-type :integer))
    ((:hn_pos_idx __hn_pos_idx) (make-array 0 :initial-element 0 :element-type :integer))
    ((:dist_to_connect_hn __dist_to_connect_hn) (make-array 0 :initial-element 0.0 :element-type :float))
    )
   (send-super :init)
   (setq _pub_t (float __pub_t))
   (setq _from_uav (round __from_uav))
   (setq _flags (round __flags))
   (setq _local_fn __local_fn)
   (setq _dist_to_local_fn __dist_to_local_fn)
   (setq _connect_fn __connect_fn)
   (setq _dist_to_connect_fn __dist_to_connect_fn)
   (setq _connect_hn __connect_hn)
   (setq _hn_pos_idx __hn_pos_idx)
   (setq _dist_to_connect_hn __dist_to_connect_hn)
   self)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:flags
   (&optional __flags)
   (if __flags (setq _flags __flags)) _flags)
  (:local_fn
   (&optional __local_fn)
   (if __local_fn (setq _local_fn __local_fn)) _local_fn)
  (:dist_to_local_fn
   (&optional __dist_to_local_fn)
   (if __dist_to_local_fn (setq _dist_to_local_fn __dist_to_local_fn)) _dist_to_local_fn)
  (:connect_fn
   (&optional __connect_fn)
   (if __connect_fn (setq _connect_fn __connect_fn)) _connect_fn)
  (:dist_to_connect_fn
   (&optional __dist_to_connect_fn)
   (if __dist_to_connect_fn (setq _dist_to_connect_fn __dist_to_connect_fn)) _dist_to_connect_fn)
  (:connect_hn
   (&optional __connect_hn)
   (if __connect_hn (setq _connect_hn __connect_hn)) _connect_hn)
  (:hn_pos_idx
   (&optional __hn_pos_idx)
   (if __hn_pos_idx (setq _hn_pos_idx __hn_pos_idx)) _hn_pos_idx)
  (:dist_to_connect_hn
   (&optional __dist_to_connect_hn)
   (if __dist_to_connect_hn (setq _dist_to_connect_hn __dist_to_connect_hn)) _dist_to_connect_hn)
  (:serialization-length
   ()
   (+
    ;; float64 _pub_t
    8
    ;; uint8 _from_uav
    1
    ;; uint8 _flags
    1
    ;; uint16[] _local_fn
    (* 2    (length _local_fn)) 4
    ;; float32[] _dist_to_local_fn
    (* 4    (length _dist_to_local_fn)) 4
    ;; uint16[] _connect_fn
    (* 2    (length _connect_fn)) 4
    ;; float32[] _dist_to_connect_fn
    (* 4    (length _dist_to_connect_fn)) 4
    ;; uint32[] _connect_hn
    (* 4    (length _connect_hn)) 4
    ;; uint32[] _hn_pos_idx
    (* 4    (length _hn_pos_idx)) 4
    ;; float32[] _dist_to_connect_hn
    (* 4    (length _dist_to_connect_hn)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; uint8 _flags
       (write-byte _flags s)
     ;; uint16[] _local_fn
     (write-long (length _local_fn) s)
     (dotimes (i (length _local_fn))
       (write-word (elt _local_fn i) s)
       )
     ;; float32[] _dist_to_local_fn
     (write-long (length _dist_to_local_fn) s)
     (dotimes (i (length _dist_to_local_fn))
       (sys::poke (elt _dist_to_local_fn i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; uint16[] _connect_fn
     (write-long (length _connect_fn) s)
     (dotimes (i (length _connect_fn))
       (write-word (elt _connect_fn i) s)
       )
     ;; float32[] _dist_to_connect_fn
     (write-long (length _dist_to_connect_fn) s)
     (dotimes (i (length _dist_to_connect_fn))
       (sys::poke (elt _dist_to_connect_fn i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;; uint32[] _connect_hn
     (write-long (length _connect_hn) s)
     (dotimes (i (length _connect_hn))
       (write-long (elt _connect_hn i) s)
       )
     ;; uint32[] _hn_pos_idx
     (write-long (length _hn_pos_idx) s)
     (dotimes (i (length _hn_pos_idx))
       (write-long (elt _hn_pos_idx i) s)
       )
     ;; float32[] _dist_to_connect_hn
     (write-long (length _dist_to_connect_hn) s)
     (dotimes (i (length _dist_to_connect_hn))
       (sys::poke (elt _dist_to_connect_hn i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _flags
     (setq _flags (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint16[] _local_fn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _local_fn (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _local_fn i) (sys::peek buf ptr- :short)) (incf ptr- 2)
     ))
   ;; float32[] _dist_to_local_fn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _dist_to_local_fn (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _dist_to_local_fn i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; uint16[] _connect_fn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _connect_fn (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _connect_fn i) (sys::peek buf ptr- :short)) (incf ptr- 2)
     ))
   ;; float32[] _dist_to_connect_fn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _dist_to_connect_fn (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _dist_to_connect_fn i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;; uint32[] _connect_hn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _connect_hn (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _connect_hn i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint32[] _hn_pos_idx
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _hn_pos_idx (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _hn_pos_idx i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; float32[] _dist_to_connect_hn
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _dist_to_connect_hn (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _dist_to_connect_hn i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;;
   self)
  )

(setf (get exp_comm_msgs::SwarmStateC :md5sum-) "11fc39f21c102f3f37a28c1e4132b63e")
(setf (get exp_comm_msgs::SwarmStateC :datatype-) "exp_comm_msgs/SwarmStateC")
(setf (get exp_comm_msgs::SwarmStateC :definition-)
      "float64 pub_t
uint8 from_uav
uint8 flags #0000 000(quick communication) 
uint16[]  local_fn
float32[] dist_to_local_fn

uint16[]  connect_fn
float32[] dist_to_connect_fn

uint32[]  connect_hn
uint32[]  hn_pos_idx
float32[] dist_to_connect_hn  


")



(provide :exp_comm_msgs/SwarmStateC "11fc39f21c102f3f37a28c1e4132b63e")


