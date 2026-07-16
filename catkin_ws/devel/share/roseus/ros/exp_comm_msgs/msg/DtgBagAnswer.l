;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::DtgBagAnswer)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'DtgBagAnswer (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::DTGBAGANSWER")
  (make-package "EXP_COMM_MSGS::DTGBAGANSWER"))

(in-package "ROS")
;;//! \htmlinclude DtgBagAnswer.msg.html


(defclass exp_comm_msgs::DtgBagAnswer
  :super ros::object
  :slots (_id _to_uav _bag_id ))

(defmethod exp_comm_msgs::DtgBagAnswer
  (:init
   (&key
    ((:id __id) 0)
    ((:to_uav __to_uav) 0)
    ((:bag_id __bag_id) 0)
    )
   (send-super :init)
   (setq _id (round __id))
   (setq _to_uav (round __to_uav))
   (setq _bag_id (round __bag_id))
   self)
  (:id
   (&optional __id)
   (if __id (setq _id __id)) _id)
  (:to_uav
   (&optional __to_uav)
   (if __to_uav (setq _to_uav __to_uav)) _to_uav)
  (:bag_id
   (&optional __bag_id)
   (if __bag_id (setq _bag_id __bag_id)) _bag_id)
  (:serialization-length
   ()
   (+
    ;; uint8 _id
    1
    ;; uint8 _to_uav
    1
    ;; uint32 _bag_id
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _id
       (write-byte _id s)
     ;; uint8 _to_uav
       (write-byte _to_uav s)
     ;; uint32 _bag_id
       (write-long _bag_id s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _id
     (setq _id (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _to_uav
     (setq _to_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint32 _bag_id
     (setq _bag_id (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get exp_comm_msgs::DtgBagAnswer :md5sum-) "38ef4e8bfd92de589cce314b18d25dfb")
(setf (get exp_comm_msgs::DtgBagAnswer :datatype-) "exp_comm_msgs/DtgBagAnswer")
(setf (get exp_comm_msgs::DtgBagAnswer :definition-)
      "uint8 id
uint8 to_uav
uint32 bag_id
")



(provide :exp_comm_msgs/DtgBagAnswer "38ef4e8bfd92de589cce314b18d25dfb")


