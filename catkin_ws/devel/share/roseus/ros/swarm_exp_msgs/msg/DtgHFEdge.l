;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgHFEdge)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgHFEdge (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGHFEDGE")
  (make-package "SWARM_EXP_MSGS::DTGHFEDGE"))

(in-package "ROS")
;;//! \htmlinclude DtgHFEdge.msg.html


(defclass swarm_exp_msgs::DtgHFEdge
  :super ros::object
  :slots (_points_idx _pub_t _f_id _h_id _vp_id ))

(defmethod swarm_exp_msgs::DtgHFEdge
  (:init
   (&key
    ((:points_idx __points_idx) (make-array 0 :initial-element 0 :element-type :integer))
    ((:pub_t __pub_t) 0.0)
    ((:f_id __f_id) 0)
    ((:h_id __h_id) 0)
    ((:vp_id __vp_id) 0)
    )
   (send-super :init)
   (setq _points_idx __points_idx)
   (setq _pub_t (float __pub_t))
   (setq _f_id (round __f_id))
   (setq _h_id (round __h_id))
   (setq _vp_id (round __vp_id))
   self)
  (:points_idx
   (&optional __points_idx)
   (if __points_idx (setq _points_idx __points_idx)) _points_idx)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:f_id
   (&optional __f_id)
   (if __f_id (setq _f_id __f_id)) _f_id)
  (:h_id
   (&optional __h_id)
   (if __h_id (setq _h_id __h_id)) _h_id)
  (:vp_id
   (&optional __vp_id)
   (if __vp_id (setq _vp_id __vp_id)) _vp_id)
  (:serialization-length
   ()
   (+
    ;; uint32[] _points_idx
    (* 4    (length _points_idx)) 4
    ;; float64 _pub_t
    8
    ;; uint16 _f_id
    2
    ;; uint32 _h_id
    4
    ;; uint8 _vp_id
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint32[] _points_idx
     (write-long (length _points_idx) s)
     (dotimes (i (length _points_idx))
       (write-long (elt _points_idx i) s)
       )
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint16 _f_id
       (write-word _f_id s)
     ;; uint32 _h_id
       (write-long _h_id s)
     ;; uint8 _vp_id
       (write-byte _vp_id s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint32[] _points_idx
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _points_idx (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _points_idx i) (sys::peek buf ptr- :integer)) (incf ptr- 4)
     ))
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint16 _f_id
     (setq _f_id (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint32 _h_id
     (setq _h_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; uint8 _vp_id
     (setq _vp_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgHFEdge :md5sum-) "e9f0be3eb0a7ce0d70ee65a834e21ddf")
(setf (get swarm_exp_msgs::DtgHFEdge :datatype-) "swarm_exp_msgs/DtgHFEdge")
(setf (get swarm_exp_msgs::DtgHFEdge :definition-)
      "uint32[] points_idx #low res map
float64 pub_t
uint16 f_id
uint32 h_id
uint8 vp_id
#uint8 hfe_id        #who creates

")



(provide :swarm_exp_msgs/DtgHFEdge "e9f0be3eb0a7ce0d70ee65a834e21ddf")


