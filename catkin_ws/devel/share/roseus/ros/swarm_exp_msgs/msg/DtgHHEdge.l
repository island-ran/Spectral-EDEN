;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgHHEdge)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgHHEdge (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGHHEDGE")
  (make-package "SWARM_EXP_MSGS::DTGHHEDGE"))

(in-package "ROS")
;;//! \htmlinclude DtgHHEdge.msg.html


(defclass swarm_exp_msgs::DtgHHEdge
  :super ros::object
  :slots (_pub_t _points_idx _head_h_id _tail_h_id ))

(defmethod swarm_exp_msgs::DtgHHEdge
  (:init
   (&key
    ((:pub_t __pub_t) 0.0)
    ((:points_idx __points_idx) (make-array 0 :initial-element 0 :element-type :integer))
    ((:head_h_id __head_h_id) 0)
    ((:tail_h_id __tail_h_id) 0)
    )
   (send-super :init)
   (setq _pub_t (float __pub_t))
   (setq _points_idx __points_idx)
   (setq _head_h_id (round __head_h_id))
   (setq _tail_h_id (round __tail_h_id))
   self)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:points_idx
   (&optional __points_idx)
   (if __points_idx (setq _points_idx __points_idx)) _points_idx)
  (:head_h_id
   (&optional __head_h_id)
   (if __head_h_id (setq _head_h_id __head_h_id)) _head_h_id)
  (:tail_h_id
   (&optional __tail_h_id)
   (if __tail_h_id (setq _tail_h_id __tail_h_id)) _tail_h_id)
  (:serialization-length
   ()
   (+
    ;; float64 _pub_t
    8
    ;; uint32[] _points_idx
    (* 4    (length _points_idx)) 4
    ;; uint32 _head_h_id
    4
    ;; uint32 _tail_h_id
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint32[] _points_idx
     (write-long (length _points_idx) s)
     (dotimes (i (length _points_idx))
       (write-long (elt _points_idx i) s)
       )
     ;; uint32 _head_h_id
       (write-long _head_h_id s)
     ;; uint32 _tail_h_id
       (write-long _tail_h_id s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint32[] _points_idx
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _points_idx (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _points_idx i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint32 _head_h_id
     (setq _head_h_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint32 _tail_h_id
     (setq _tail_h_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgHHEdge :md5sum-) "f28c77b458771baddef5547b4e2a8c2d")
(setf (get swarm_exp_msgs::DtgHHEdge :datatype-) "swarm_exp_msgs/DtgHHEdge")
(setf (get swarm_exp_msgs::DtgHHEdge :definition-)
      "#uint8 hhe_id        #who creates
#bool erase 
float64 pub_t
uint32[] points_idx #low res map
uint32 head_h_id
uint32 tail_h_id
")



(provide :swarm_exp_msgs/DtgHHEdge "f28c77b458771baddef5547b4e2a8c2d")


