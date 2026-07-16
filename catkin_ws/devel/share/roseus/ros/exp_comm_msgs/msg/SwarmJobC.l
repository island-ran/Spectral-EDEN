;; Auto-generated. Do not edit!


(when (boundp 'exp_comm_msgs::SwarmJobC)
  (if (not (find-package "EXP_COMM_MSGS"))
    (make-package "EXP_COMM_MSGS"))
  (shadow 'SwarmJobC (find-package "EXP_COMM_MSGS")))
(unless (find-package "EXP_COMM_MSGS::SWARMJOBC")
  (make-package "EXP_COMM_MSGS::SWARMJOBC"))

(in-package "ROS")
;;//! \htmlinclude SwarmJobC.msg.html


(defclass exp_comm_msgs::SwarmJobC
  :super ros::object
  :slots (_pub_t _from_uav _JobState _target_fn _target_hn _dist_to_fn _dist_to_hn ))

(defmethod exp_comm_msgs::SwarmJobC
  (:init
   (&key
    ((:pub_t __pub_t) 0.0)
    ((:from_uav __from_uav) 0)
    ((:JobState __JobState) 0)
    ((:target_fn __target_fn) 0)
    ((:target_hn __target_hn) 0)
    ((:dist_to_fn __dist_to_fn) 0.0)
    ((:dist_to_hn __dist_to_hn) 0.0)
    )
   (send-super :init)
   (setq _pub_t (float __pub_t))
   (setq _from_uav (round __from_uav))
   (setq _JobState (round __JobState))
   (setq _target_fn (round __target_fn))
   (setq _target_hn (round __target_hn))
   (setq _dist_to_fn (float __dist_to_fn))
   (setq _dist_to_hn (float __dist_to_hn))
   self)
  (:pub_t
   (&optional __pub_t)
   (if __pub_t (setq _pub_t __pub_t)) _pub_t)
  (:from_uav
   (&optional __from_uav)
   (if __from_uav (setq _from_uav __from_uav)) _from_uav)
  (:JobState
   (&optional __JobState)
   (if __JobState (setq _JobState __JobState)) _JobState)
  (:target_fn
   (&optional __target_fn)
   (if __target_fn (setq _target_fn __target_fn)) _target_fn)
  (:target_hn
   (&optional __target_hn)
   (if __target_hn (setq _target_hn __target_hn)) _target_hn)
  (:dist_to_fn
   (&optional __dist_to_fn)
   (if __dist_to_fn (setq _dist_to_fn __dist_to_fn)) _dist_to_fn)
  (:dist_to_hn
   (&optional __dist_to_hn)
   (if __dist_to_hn (setq _dist_to_hn __dist_to_hn)) _dist_to_hn)
  (:serialization-length
   ()
   (+
    ;; float64 _pub_t
    8
    ;; uint8 _from_uav
    1
    ;; uint8 _JobState
    1
    ;; int32 _target_fn
    4
    ;; int32 _target_hn
    4
    ;; float32 _dist_to_fn
    4
    ;; float32 _dist_to_hn
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _pub_t
       (sys::poke _pub_t (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; uint8 _from_uav
       (write-byte _from_uav s)
     ;; uint8 _JobState
       (write-byte _JobState s)
     ;; int32 _target_fn
       (write-long _target_fn s)
     ;; int32 _target_hn
       (write-long _target_hn s)
     ;; float32 _dist_to_fn
       (sys::poke _dist_to_fn (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _dist_to_hn
       (sys::poke _dist_to_hn (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _pub_t
     (setq _pub_t (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; uint8 _from_uav
     (setq _from_uav (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _JobState
     (setq _JobState (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; int32 _target_fn
     (setq _target_fn (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; int32 _target_hn
     (setq _target_hn (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; float32 _dist_to_fn
     (setq _dist_to_fn (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _dist_to_hn
     (setq _dist_to_hn (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get exp_comm_msgs::SwarmJobC :md5sum-) "e25c09eda529caf2748eb82a8886575b")
(setf (get exp_comm_msgs::SwarmJobC :datatype-) "exp_comm_msgs/SwarmJobC")
(setf (get exp_comm_msgs::SwarmJobC :definition-)
      "float64 pub_t
uint8 from_uav
uint8 JobState #0000 0(no job = 0/ new job = 1)(finish = 1)(local = 0/ global = 1)
int32  target_fn
int32 target_hn
float32 dist_to_fn
float32 dist_to_hn



")



(provide :exp_comm_msgs/SwarmJobC "e25c09eda529caf2748eb82a8886575b")


