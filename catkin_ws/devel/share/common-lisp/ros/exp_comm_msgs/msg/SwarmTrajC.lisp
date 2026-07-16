; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude SwarmTrajC.msg.html

(cl:defclass <SwarmTrajC> (roslisp-msg-protocol:ros-message)
  ((pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
   (id
    :reader id
    :initarg :id
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

(cl:defclass SwarmTrajC (<SwarmTrajC>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SwarmTrajC>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SwarmTrajC)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<SwarmTrajC> is deprecated: use exp_comm_msgs-msg:SwarmTrajC instead.")))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:pub_t-val is deprecated.  Use exp_comm_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:id-val is deprecated.  Use exp_comm_msgs-msg:id instead.")
  (id m))

(cl:ensure-generic-function 'start_t-val :lambda-list '(m))
(cl:defmethod start_t-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:start_t-val is deprecated.  Use exp_comm_msgs-msg:start_t instead.")
  (start_t m))

(cl:ensure-generic-function 'order_p-val :lambda-list '(m))
(cl:defmethod order_p-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:order_p-val is deprecated.  Use exp_comm_msgs-msg:order_p instead.")
  (order_p m))

(cl:ensure-generic-function 't_p-val :lambda-list '(m))
(cl:defmethod t_p-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:t_p-val is deprecated.  Use exp_comm_msgs-msg:t_p instead.")
  (t_p m))

(cl:ensure-generic-function 'coef_p-val :lambda-list '(m))
(cl:defmethod coef_p-val ((m <SwarmTrajC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:coef_p-val is deprecated.  Use exp_comm_msgs-msg:coef_p instead.")
  (coef_p m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SwarmTrajC>) ostream)
  "Serializes a message object of type '<SwarmTrajC>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) ostream)
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
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SwarmTrajC>) istream)
  "Deserializes a message object of type '<SwarmTrajC>"
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) (cl:read-byte istream))
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SwarmTrajC>)))
  "Returns string type for a message object of type '<SwarmTrajC>"
  "exp_comm_msgs/SwarmTrajC")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SwarmTrajC)))
  "Returns string type for a message object of type 'SwarmTrajC"
  "exp_comm_msgs/SwarmTrajC")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SwarmTrajC>)))
  "Returns md5sum for a message object of type '<SwarmTrajC>"
  "b412e83c14c239c5f9f2ca748f0bead0")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SwarmTrajC)))
  "Returns md5sum for a message object of type 'SwarmTrajC"
  "b412e83c14c239c5f9f2ca748f0bead0")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SwarmTrajC>)))
  "Returns full string definition for message of type '<SwarmTrajC>"
  (cl:format cl:nil "float64 pub_t~%~%uint8 id~%~%float64 start_t~%int8 order_p~%float32[] t_p~%geometry_msgs/Point[] coef_p~%~%# int8 order_yaw~%# float32[] t_yaw~%# float32[] coef_yaw~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SwarmTrajC)))
  "Returns full string definition for message of type 'SwarmTrajC"
  (cl:format cl:nil "float64 pub_t~%~%uint8 id~%~%float64 start_t~%int8 order_p~%float32[] t_p~%geometry_msgs/Point[] coef_p~%~%# int8 order_yaw~%# float32[] t_yaw~%# float32[] coef_yaw~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SwarmTrajC>))
  (cl:+ 0
     8
     1
     8
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 't_p) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'coef_p) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SwarmTrajC>))
  "Converts a ROS message object to a list"
  (cl:list 'SwarmTrajC
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':id (id msg))
    (cl:cons ':start_t (start_t msg))
    (cl:cons ':order_p (order_p msg))
    (cl:cons ':t_p (t_p msg))
    (cl:cons ':coef_p (coef_p msg))
))
