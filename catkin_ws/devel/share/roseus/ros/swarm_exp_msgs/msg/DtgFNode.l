;; Auto-generated. Do not edit!


(when (boundp 'swarm_exp_msgs::DtgFNode)
  (if (not (find-package "SWARM_EXP_MSGS"))
    (make-package "SWARM_EXP_MSGS"))
  (shadow 'DtgFNode (find-package "SWARM_EXP_MSGS")))
(unless (find-package "SWARM_EXP_MSGS::DTGFNODE")
  (make-package "SWARM_EXP_MSGS::DTGFNODE"))

(in-package "ROS")
;;//! \htmlinclude DtgFNode.msg.html


(defclass swarm_exp_msgs::DtgFNode
  :super ros::object
  :slots (_f_id _alive _vp_flags _need_help ))

(defmethod swarm_exp_msgs::DtgFNode
  (:init
   (&key
    ((:f_id __f_id) 0)
    ((:alive __alive) nil)
    ((:vp_flags __vp_flags) (make-array 0 :initial-element 0 :element-type :char))
    ((:need_help __need_help) 0)
    )
   (send-super :init)
   (setq _f_id (round __f_id))
   (setq _alive __alive)
   (setq _vp_flags __vp_flags)
   (setq _need_help (round __need_help))
   self)
  (:f_id
   (&optional __f_id)
   (if __f_id (setq _f_id __f_id)) _f_id)
  (:alive
   (&optional (__alive :null))
   (if (not (eq __alive :null)) (setq _alive __alive)) _alive)
  (:vp_flags
   (&optional __vp_flags)
   (if __vp_flags (setq _vp_flags __vp_flags)) _vp_flags)
  (:need_help
   (&optional __need_help)
   (if __need_help (setq _need_help __need_help)) _need_help)
  (:serialization-length
   ()
   (+
    ;; uint16 _f_id
    2
    ;; bool _alive
    1
    ;; uint8[] _vp_flags
    (* 1    (length _vp_flags)) 4
    ;; uint8 _need_help
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint16 _f_id
       (write-word _f_id s)
     ;; bool _alive
       (if _alive (write-byte -1 s) (write-byte 0 s))
     ;; uint8[] _vp_flags
     (write-long (length _vp_flags) s)
     (princ _vp_flags s)
     ;; uint8 _need_help
       (write-byte _need_help s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint16 _f_id
     (setq _f_id (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; bool _alive
     (setq _alive (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint8[] _vp_flags
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _vp_flags (make-array n :element-type :char))
     (replace _vp_flags buf :start2 ptr-) (incf ptr- n))
   ;; uint8 _need_help
     (setq _need_help (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get swarm_exp_msgs::DtgFNode :md5sum-) "f35023aa28359878229c76466f958b4c")
(setf (get swarm_exp_msgs::DtgFNode :datatype-) "swarm_exp_msgs/DtgFNode")
(setf (get swarm_exp_msgs::DtgFNode :definition-)
      "uint16 f_id
bool alive
uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)
uint8 need_help # ask other uav for edge

")



(provide :swarm_exp_msgs/DtgFNode "f35023aa28359878229c76466f958b4c")


