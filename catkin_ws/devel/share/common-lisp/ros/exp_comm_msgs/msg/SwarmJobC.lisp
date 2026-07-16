; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude SwarmJobC.msg.html

(cl:defclass <SwarmJobC> (roslisp-msg-protocol:ros-message)
  ((pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
   (from_uav
    :reader from_uav
    :initarg :from_uav
    :type cl:fixnum
    :initform 0)
   (JobState
    :reader JobState
    :initarg :JobState
    :type cl:fixnum
    :initform 0)
   (target_fn
    :reader target_fn
    :initarg :target_fn
    :type cl:integer
    :initform 0)
   (target_hn
    :reader target_hn
    :initarg :target_hn
    :type cl:integer
    :initform 0)
   (dist_to_fn
    :reader dist_to_fn
    :initarg :dist_to_fn
    :type cl:float
    :initform 0.0)
   (dist_to_hn
    :reader dist_to_hn
    :initarg :dist_to_hn
    :type cl:float
    :initform 0.0))
)

(cl:defclass SwarmJobC (<SwarmJobC>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SwarmJobC>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SwarmJobC)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<SwarmJobC> is deprecated: use exp_comm_msgs-msg:SwarmJobC instead.")))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:pub_t-val is deprecated.  Use exp_comm_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:from_uav-val is deprecated.  Use exp_comm_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'JobState-val :lambda-list '(m))
(cl:defmethod JobState-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:JobState-val is deprecated.  Use exp_comm_msgs-msg:JobState instead.")
  (JobState m))

(cl:ensure-generic-function 'target_fn-val :lambda-list '(m))
(cl:defmethod target_fn-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:target_fn-val is deprecated.  Use exp_comm_msgs-msg:target_fn instead.")
  (target_fn m))

(cl:ensure-generic-function 'target_hn-val :lambda-list '(m))
(cl:defmethod target_hn-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:target_hn-val is deprecated.  Use exp_comm_msgs-msg:target_hn instead.")
  (target_hn m))

(cl:ensure-generic-function 'dist_to_fn-val :lambda-list '(m))
(cl:defmethod dist_to_fn-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:dist_to_fn-val is deprecated.  Use exp_comm_msgs-msg:dist_to_fn instead.")
  (dist_to_fn m))

(cl:ensure-generic-function 'dist_to_hn-val :lambda-list '(m))
(cl:defmethod dist_to_hn-val ((m <SwarmJobC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:dist_to_hn-val is deprecated.  Use exp_comm_msgs-msg:dist_to_hn instead.")
  (dist_to_hn m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SwarmJobC>) ostream)
  "Serializes a message object of type '<SwarmJobC>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'JobState)) ostream)
  (cl:let* ((signed (cl:slot-value msg 'target_fn)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'target_hn)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 4294967296) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) unsigned) ostream)
    )
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'dist_to_fn))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'dist_to_hn))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SwarmJobC>) istream)
  "Deserializes a message object of type '<SwarmJobC>"
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'pub_t) (roslisp-utils:decode-double-float-bits bits)))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'JobState)) (cl:read-byte istream))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'target_fn) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'target_hn) (cl:if (cl:< unsigned 2147483648) unsigned (cl:- unsigned 4294967296))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'dist_to_fn) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'dist_to_hn) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SwarmJobC>)))
  "Returns string type for a message object of type '<SwarmJobC>"
  "exp_comm_msgs/SwarmJobC")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SwarmJobC)))
  "Returns string type for a message object of type 'SwarmJobC"
  "exp_comm_msgs/SwarmJobC")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SwarmJobC>)))
  "Returns md5sum for a message object of type '<SwarmJobC>"
  "e25c09eda529caf2748eb82a8886575b")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SwarmJobC)))
  "Returns md5sum for a message object of type 'SwarmJobC"
  "e25c09eda529caf2748eb82a8886575b")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SwarmJobC>)))
  "Returns full string definition for message of type '<SwarmJobC>"
  (cl:format cl:nil "float64 pub_t~%uint8 from_uav~%uint8 JobState #0000 0(no job = 0/ new job = 1)(finish = 1)(local = 0/ global = 1)~%int32  target_fn~%int32 target_hn~%float32 dist_to_fn~%float32 dist_to_hn~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SwarmJobC)))
  "Returns full string definition for message of type 'SwarmJobC"
  (cl:format cl:nil "float64 pub_t~%uint8 from_uav~%uint8 JobState #0000 0(no job = 0/ new job = 1)(finish = 1)(local = 0/ global = 1)~%int32  target_fn~%int32 target_hn~%float32 dist_to_fn~%float32 dist_to_hn~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SwarmJobC>))
  (cl:+ 0
     8
     1
     1
     4
     4
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SwarmJobC>))
  "Converts a ROS message object to a list"
  (cl:list 'SwarmJobC
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':JobState (JobState msg))
    (cl:cons ':target_fn (target_fn msg))
    (cl:cons ':target_hn (target_hn msg))
    (cl:cons ':dist_to_fn (dist_to_fn msg))
    (cl:cons ':dist_to_hn (dist_to_hn msg))
))
