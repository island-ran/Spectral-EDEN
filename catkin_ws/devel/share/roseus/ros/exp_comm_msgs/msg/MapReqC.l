;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::MapReqC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'MapReqC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::MAPREQC")
  (make-package "EXP_COMM_MSGS::MAPREQC"))

(in-package "ROS")
;;//! \htmlinclude MapReqC.msg.html


(defclass exp_comm_msgs::MapReqC
  :super ros::object
  :slots (_f_id _block_id _flag ))

(defmethod exp_comm_msgs::MapReqC
  (:init
   (&key
    ((:f_id __f_id) (make-array 0 :initial-element 0 :element-type :integer))
    ((:block_id __block_id) (make-array 0 :initial-element 0 :element-type :char))
    ((:flag __flag) 0)
    )
   (send-super :init)
   (setq _f_id __f_id)
   (setq _block_id __block_id)
   (setq _flag (round __flag))
   self)
  (:f_id
   (&optional __f_id)
   (if __f_id (setq _f_id __f_id)) _f_id)
  (:block_id
   (&optional __block_id)
   (if __block_id (setq _block_id __block_id)) _block_id)
  (:flag
   (&optional __flag)
   (if __flag (setq _flag __flag)) _flag)
  (:serialization-length
   ()
   (+
    ;; uint16[] _f_id
    (* 2    (length _f_id)) 4
    ;; uint8[] _block_id
    (* 1    (length _block_id)) 4
    ;; uint8 _flag
    1
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint16[] _f_id
     (write-long (length _f_id) s)
     (dotimes (i (length _f_id))
       (write-word (elt _f_id i) s)
       )
     ;; uint8[] _block_id
     (write-long (length _block_id) s)
     (princ _block_id s)
     ;; uint8 _flag
       (write-byte _flag s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint16[] _f_id
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _f_id (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _f_id i) (sys::peek buf ptr- :short)) (incf ptr- 2)
     ))
   ;; uint8[] _block_id
   (let ((n (sys::peek buf ptr- :integer))) (incf ptr- 4)
     (setq _block_id (make-array n :element-type :char))
     (replace _block_id buf :start2 ptr-) (incf ptr- n))
   ;; uint8 _flag
     (setq _flag (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;;
   self)
  )

(setf (get exp_comm_msgs::MapReqC :md5sum-) "71af631c49d7e61ef9c70b1b03b18a05")
(setf (get exp_comm_msgs::MapReqC :datatype-) "exp_comm_msgs/MapReqC")
(setf (get exp_comm_msgs::MapReqC :definition-)
      "uint16[] f_id
uint8[] block_id
uint8 flag
")



(provide :exp_comm_msgs/MapReqC "71af631c49d7e61ef9c70b1b03b18a05")


