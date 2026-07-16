; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude MapReqC.msg.html

(cl:defclass <MapReqC> (roslisp-msg-protocol:ros-message)
  ((f_id
    :reader f_id
    :initarg :f_id
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (block_id
    :reader block_id
    :initarg :block_id
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (flag
    :reader flag
    :initarg :flag
    :type cl:fixnum
    :initform 0))
)

(cl:defclass MapReqC (<MapReqC>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <MapReqC>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'MapReqC)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<MapReqC> is deprecated: use exp_comm_msgs-msg:MapReqC instead.")))

(cl:ensure-generic-function 'f_id-val :lambda-list '(m))
(cl:defmethod f_id-val ((m <MapReqC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:f_id-val is deprecated.  Use exp_comm_msgs-msg:f_id instead.")
  (f_id m))

(cl:ensure-generic-function 'block_id-val :lambda-list '(m))
(cl:defmethod block_id-val ((m <MapReqC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:block_id-val is deprecated.  Use exp_comm_msgs-msg:block_id instead.")
  (block_id m))

(cl:ensure-generic-function 'flag-val :lambda-list '(m))
(cl:defmethod flag-val ((m <MapReqC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:flag-val is deprecated.  Use exp_comm_msgs-msg:flag instead.")
  (flag m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <MapReqC>) ostream)
  "Serializes a message object of type '<MapReqC>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'f_id))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream))
   (cl:slot-value msg 'f_id))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'block_id))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'block_id))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'flag)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <MapReqC>) istream)
  "Deserializes a message object of type '<MapReqC>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'f_id) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'f_id)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream)))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'block_id) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'block_id)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'flag)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<MapReqC>)))
  "Returns string type for a message object of type '<MapReqC>"
  "exp_comm_msgs/MapReqC")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'MapReqC)))
  "Returns string type for a message object of type 'MapReqC"
  "exp_comm_msgs/MapReqC")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<MapReqC>)))
  "Returns md5sum for a message object of type '<MapReqC>"
  "71af631c49d7e61ef9c70b1b03b18a05")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'MapReqC)))
  "Returns md5sum for a message object of type 'MapReqC"
  "71af631c49d7e61ef9c70b1b03b18a05")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<MapReqC>)))
  "Returns full string definition for message of type '<MapReqC>"
  (cl:format cl:nil "uint16[] f_id~%uint8[] block_id~%uint8 flag~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'MapReqC)))
  "Returns full string definition for message of type 'MapReqC"
  (cl:format cl:nil "uint16[] f_id~%uint8[] block_id~%uint8 flag~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <MapReqC>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'f_id) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 2)))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'block_id) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <MapReqC>))
  "Converts a ROS message object to a list"
  (cl:list 'MapReqC
    (cl:cons ':f_id (f_id msg))
    (cl:cons ':block_id (block_id msg))
    (cl:cons ':flag (flag msg))
))
