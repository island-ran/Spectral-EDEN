;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgFFEdge)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgFFEdge (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGFFEDGE")
  (make-package "SWARM_EXP_MSGS::DTGFFEDGE"))

(in-package "ROS")
;;//! \htmlinclude DtgFFEdge.msg.html


(defclass swarm_exp_msgs::DtgFFEdge
  :super ros::object
  :slots (_head_f_id _tail_f_id _points_idx _head_v_id _tail_v_id _ffe_id ))

(defmethod swarm_exp_msgs::DtgFFEdge
  (:init
   (&key
    ((:head_f_id __head_f_id) 0)
    ((:tail_f_id __tail_f_id) 0)
    ((:points_idx __points_idx) (make-array 0 :initial-element 0 :element-type :integer))
    ((:head_v_id __head_v_id) 0)
    ((:tail_v_id __tail_v_id) 0)
    ((:ffe_id __ffe_id) 0)
    )
   (send-super :init)
   (setq _head_f_id (round __head_f_id))
   (setq _tail_f_id (round __tail_f_id))
   (setq _points_idx __points_idx)
   (setq _head_v_id (round __head_v_id))
   (setq _tail_v_id (round __tail_v_id))
   (setq _ffe_id (round __ffe_id))
   self)
  (:head_f_id
   (&optional __head_f_id)
   (if __head_f_id (setq _head_f_id __head_f_id)) _head_f_id)
  (:tail_f_id
   (&optional __tail_f_id)
   (if __tail_f_id (setq _tail_f_id __tail_f_id)) _tail_f_id)
  (:points_idx
   (&optional __points_idx)
   (if __points_idx (setq _points_idx __points_idx)) _points_idx)
  (:head_v_id
   (&optional __head_v_id)
   (if __head_v_id (setq _head_v_id __head_v_id)) _head_v_id)
  (:tail_v_id
   (&optional __tail_v_id)
   (if __tail_v_id (setq _tail_v_id __tail_v_id)) _tail_v_id)
  (:ffe_id
   (&optional __ffe_id)
   (if __ffe_id (setq _ffe_id __ffe_id)) _ffe_id)
  (:serialization-length
   ()
   (+
    ;; uint16 _head_f_id
    2
    ;; uint16 _tail_f_id
    2
    ;; uint32[] _points_idx
    (* 4    (length _points_idx)) 4
    ;; uint8 _head_v_id
    1
    ;; uint8 _tail_v_id
    1
    ;; uint8 _ffe_id
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint16 _head_f_id
       (write-word _head_f_id s)
     ;; uint16 _tail_f_id
       (write-word _tail_f_id s)
     ;; uint32[] _points_idx
     (write-long (length _points_idx) s)
     (dotimes (i (length _points_idx))
       (write-long (elt _points_idx i) s)
       )
     ;; uint8 _head_v_id
       (write-byte _head_v_id s)
     ;; uint8 _tail_v_id
       (write-byte _tail_v_id s)
     ;; uint8 _ffe_id
       (write-byte _ffe_id s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint16 _head_f_id
     (setq _head_f_id (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint16 _tail_f_id
     (setq _tail_f_id (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint32[] _points_idx
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _points_idx (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _points_idx i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; uint8 _head_v_id
     (setq _head_v_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _tail_v_id
     (setq _tail_v_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _ffe_id
     (setq _ffe_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgFFEdge :md5sum-) "764ef8eb22d76355c9c291bdaa1ae595")
(setf (get swarm_exp_msgs::DtgFFEdge :datatype-) "swarm_exp_msgs/DtgFFEdge")
(setf (get swarm_exp_msgs::DtgFFEdge :definition-)
      "uint16 head_f_id
uint16 tail_f_id
uint32[] points_idx #low res map

uint8 head_v_id #viewpoint id of head fn
uint8 tail_v_id #viewpoint id of tail fn
uint8 ffe_id    #who creates

")



(provide :swarm_exp_msgs/DtgFFEdge "764ef8eb22d76355c9c291bdaa1ae595")


