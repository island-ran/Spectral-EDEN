;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::MapC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'MapC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::MAPC")
  (make-package "EXP_COMM_MSGS::MAPC"))

(in-package "ROS")
;;//! \htmlinclude MapC.msg.html


(defclass exp_comm_msgs::MapC
  :super ros::object
  :slots (_f_id _block_id _block_state _flags ))

(defmethod exp_comm_msgs::MapC
  (:init
   (&key
    ((:f_id __f_id) 0)
    ((:block_id __block_id) 0)
    ((:block_state __block_state) 0)
    ((:flags __flags) (make-array 0 :initial-element 0 :element-type :char))
    )
   (send-super :init)
   (setq _f_id (round __f_id))
   (setq _block_id (round __block_id))
   (setq _block_state (round __block_state))
   (setq _flags __flags)
   self)
  (:f_id
   (&optional __f_id)
   (if __f_id (setq _f_id __f_id)) _f_id)
  (:block_id
   (&optional __block_id)
   (if __block_id (setq _block_id __block_id)) _block_id)
  (:block_state
   (&optional __block_state)
   (if __block_state (setq _block_state __block_state)) _block_state)
  (:flags
   (&optional __flags)
   (if __flags (setq _flags __flags)) _flags)
  (:serialization-length
   ()
   (+
    ;; uint16 _f_id
    2
    ;; uint8 _block_id
    1
    ;; uint8 _block_state
    1
    ;; uint8[] _flags
    (* 1    (length _flags)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint16 _f_id
       (write-word _f_id s)
     ;; uint8 _block_id
       (write-byte _block_id s)
     ;; uint8 _block_state
       (write-byte _block_state s)
     ;; uint8[] _flags
     (write-long (length _flags) s)
     (princ _flags s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint16 _f_id
     (setq _f_id (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; uint8 _block_id
     (setq _block_id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _block_state
     (setq _block_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8[] _flags
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _flags (make-array n :element-type :char))
     (replace _flags buf :start2 ptr-) (incf ptr- n))
   ;;
   self)
  )

(setf (get exp_comm_msgs::MapC :md5sum-) "f846807ed51661abe48fa122b2e37650")
(setf (get exp_comm_msgs::MapC :datatype-) "exp_comm_msgs/MapC")
(setf (get exp_comm_msgs::MapC :definition-)
      "uint16 f_id
uint8 block_id #1-8
uint8 block_state #0: unknown 1:occ 2:free 3:mix
uint8[] flags #00 00 00 00  (free occ)(free occ)(free occ)(free occ)
")



(provide :exp_comm_msgs/MapC "f846807ed51661abe48fa122b2e37650")


