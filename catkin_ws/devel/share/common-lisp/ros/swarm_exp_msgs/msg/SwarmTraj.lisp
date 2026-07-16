; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude SwarmTraj.msg.html

(cl:defclass <SwarmTraj> (roslisp-msg-protocol:ros-message)
  ((to_uavs
    :reader to_uavs
    :initarg :to_uavs
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (from_uav
    :reader from_uav
    :initarg :from_uav
    :type cl:fixnum
    :initform 0)
   (start_t
    :reader start_t
    :initarg :start_t
    :type cl:float
    :initform 0.0)
   (order_p
    :reader order_p
    :initarg :order_p
    :type cl:fixnum
    :initform 0)
   (t_p
    :reader t_p
    :initarg :t_p
    :type (cl:vector cl:float)
   :initform (cl:make-array 0 :element-type 'cl:float :initial-element 0.0))
   (coef_p
    :reader coef_p
    :initarg :coef_p
    :type (cl:vector geometry_msgs-msg:Point)
   :initform (cl:make-array 0 :element-type 'geometry_msgs-msg:Point :initial-element (cl:make-instance 'geometry_msgs-msg:Point))))
)

(cl:defclass SwarmTraj (<SwarmTraj>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SwarmTraj>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SwarmTraj)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<SwarmTraj> is deprecated: use swarm_exp_msgs-msg:SwarmTraj instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'start_t-val :lambda-list '(m))
(cl:defmethod start_t-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:start_t-val is deprecated.  Use swarm_exp_msgs-msg:start_t instead.")
  (start_t m))

(cl:ensure-generic-function 'order_p-val :lambda-list '(m))
(cl:defmethod order_p-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:order_p-val is deprecated.  Use swarm_exp_msgs-msg:order_p instead.")
  (order_p m))

(cl:ensure-generic-function 't_p-val :lambda-list '(m))
(cl:defmethod t_p-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:t_p-val is deprecated.  Use swarm_exp_msgs-msg:t_p instead.")
  (t_p m))

(cl:ensure-generic-function 'coef_p-val :lambda-list '(m))
(cl:defmethod coef_p-val ((m <SwarmTraj>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:coef_p-val is deprecated.  Use swarm_exp_msgs-msg:coef_p instead.")
  (coef_p m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SwarmTraj>) ostream)
  "Serializes a message object of type '<SwarmTraj>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'start_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let* ((signed (cl:slot-value msg 'order_p)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 256) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    )
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 't_p))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-single-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)))
   (cl:slot-value msg 't_p))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'coef_p))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'coef_p))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SwarmTraj>) istream)
  "Deserializes a message object of type '<SwarmTraj>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'to_uavs) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'to_uavs)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 32) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 40) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 48) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 56) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'start_t) (roslisp-utils:decode-double-float-bits bits)))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'order_p) (cl:if (cl:< unsigned 128) unsigned (cl:- unsigned 256))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 't_p) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 't_p)))
    (cl:dotimes (i __ros_arr_len)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-single-float-bits bits))))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'coef_p) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'coef_p)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'geometry_msgs-msg:Point))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SwarmTraj>)))
  "Returns string type for a message object of type '<SwarmTraj>"
  "swarm_exp_msgs/SwarmTraj")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SwarmTraj)))
  "Returns string type for a message object of type 'SwarmTraj"
  "swarm_exp_msgs/SwarmTraj")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SwarmTraj>)))
  "Returns md5sum for a message object of type '<SwarmTraj>"
  "ad6c481b3a1cd3eae78b78f76fc6821d")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SwarmTraj)))
  "Returns md5sum for a message object of type 'SwarmTraj"
  "ad6c481b3a1cd3eae78b78f76fc6821d")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SwarmTraj>)))
  "Returns full string definition for message of type '<SwarmTraj>"
  (cl:format cl:nil "# immediate for local or group~%~%uint8[] to_uavs~%uint8 from_uav~%~%float64 start_t~%int8 order_p~%float32[] t_p~%geometry_msgs/Point[] coef_p~%~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SwarmTraj)))
  "Returns full string definition for message of type 'SwarmTraj"
  (cl:format cl:nil "# immediate for local or group~%~%uint8[] to_uavs~%uint8 from_uav~%~%float64 start_t~%int8 order_p~%float32[] t_p~%geometry_msgs/Point[] coef_p~%~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SwarmTraj>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     8
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 't_p) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'coef_p) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SwarmTraj>))
  "Converts a ROS message object to a list"
  (cl:list 'SwarmTraj
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':start_t (start_t msg))
    (cl:cons ':order_p (order_p msg))
    (cl:cons ':t_p (t_p msg))
    (cl:cons ':coef_p (coef_p msg))
))
