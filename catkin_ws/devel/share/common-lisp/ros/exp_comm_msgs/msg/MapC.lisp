; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude MapC.msg.html

(cl:defclass <MapC> (roslisp-msg-protocol:ros-message)
  ((f_id
    :reader f_id
    :initarg :f_id
    :type cl:fixnum
    :initform 0)
   (block_id
    :reader block_id
    :initarg :block_id
    :type cl:fixnum
    :initform 0)
   (block_state
    :reader block_state
    :initarg :block_state
    :type cl:fixnum
    :initform 0)
   (flags
    :reader flags
    :initarg :flags
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0)))
)

(cl:defclass MapC (<MapC>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <MapC>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'MapC)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<MapC> is deprecated: use exp_comm_msgs-msg:MapC instead.")))

(cl:ensure-generic-function 'f_id-val :lambda-list '(m))
(cl:defmethod f_id-val ((m <MapC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:f_id-val is deprecated.  Use exp_comm_msgs-msg:f_id instead.")
  (f_id m))

(cl:ensure-generic-function 'block_id-val :lambda-list '(m))
(cl:defmethod block_id-val ((m <MapC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:block_id-val is deprecated.  Use exp_comm_msgs-msg:block_id instead.")
  (block_id m))

(cl:ensure-generic-function 'block_state-val :lambda-list '(m))
(cl:defmethod block_state-val ((m <MapC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:block_state-val is deprecated.  Use exp_comm_msgs-msg:block_state instead.")
  (block_state m))

(cl:ensure-generic-function 'flags-val :lambda-list '(m))
(cl:defmethod flags-val ((m <MapC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:flags-val is deprecated.  Use exp_comm_msgs-msg:flags instead.")
  (flags m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <MapC>) ostream)
  "Serializes a message object of type '<MapC>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'block_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'block_state)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'flags))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'flags))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <MapC>) istream)
  "Deserializes a message object of type '<MapC>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'block_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'block_state)) (cl:read-byte istream))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'flags) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'flags)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<MapC>)))
  "Returns string type for a message object of type '<MapC>"
  "exp_comm_msgs/MapC")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'MapC)))
  "Returns string type for a message object of type 'MapC"
  "exp_comm_msgs/MapC")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<MapC>)))
  "Returns md5sum for a message object of type '<MapC>"
  "f846807ed51661abe48fa122b2e37650")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'MapC)))
  "Returns md5sum for a message object of type 'MapC"
  "f846807ed51661abe48fa122b2e37650")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<MapC>)))
  "Returns full string definition for message of type '<MapC>"
  (cl:format cl:nil "uint16 f_id~%uint8 block_id #1-8~%uint8 block_state #0: unknown 1:occ 2:free 3:mix~%uint8[] flags #00 00 00 00  (free occ)(free occ)(free occ)(free occ)~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'MapC)))
  "Returns full string definition for message of type 'MapC"
  (cl:format cl:nil "uint16 f_id~%uint8 block_id #1-8~%uint8 block_state #0: unknown 1:occ 2:free 3:mix~%uint8[] flags #00 00 00 00  (free occ)(free occ)(free occ)(free occ)~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <MapC>))
  (cl:+ 0
     2
     1
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'flags) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <MapC>))
  "Converts a ROS message object to a list"
  (cl:list 'MapC
    (cl:cons ':f_id (f_id msg))
    (cl:cons ':block_id (block_id msg))
    (cl:cons ':block_state (block_state msg))
    (cl:cons ':flags (flags msg))
))
