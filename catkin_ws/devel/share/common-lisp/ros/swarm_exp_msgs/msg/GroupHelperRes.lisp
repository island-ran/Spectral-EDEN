; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupHelperRes.msg.html

(cl:defclass <GroupHelperRes> (roslisp-msg-protocol:ros-message)
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
   (allowed_num
    :reader allowed_num
    :initarg :allowed_num
    :type cl:fixnum
    :initform 0))
)

(cl:defclass GroupHelperRes (<GroupHelperRes>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupHelperRes>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupHelperRes)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupHelperRes> is deprecated: use swarm_exp_msgs-msg:GroupHelperRes instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <GroupHelperRes>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <GroupHelperRes>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'allowed_num-val :lambda-list '(m))
(cl:defmethod allowed_num-val ((m <GroupHelperRes>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:allowed_num-val is deprecated.  Use swarm_exp_msgs-msg:allowed_num instead.")
  (allowed_num m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupHelperRes>) ostream)
  "Serializes a message object of type '<GroupHelperRes>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'allowed_num)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupHelperRes>) istream)
  "Deserializes a message object of type '<GroupHelperRes>"
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'allowed_num)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupHelperRes>)))
  "Returns string type for a message object of type '<GroupHelperRes>"
  "swarm_exp_msgs/GroupHelperRes")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupHelperRes)))
  "Returns string type for a message object of type 'GroupHelperRes"
  "swarm_exp_msgs/GroupHelperRes")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupHelperRes>)))
  "Returns md5sum for a message object of type '<GroupHelperRes>"
  "f1d5958dfe9e4c0dcd170ba5dda86e59")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupHelperRes)))
  "Returns md5sum for a message object of type 'GroupHelperRes"
  "f1d5958dfe9e4c0dcd170ba5dda86e59")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupHelperRes>)))
  "Returns full string definition for message of type '<GroupHelperRes>"
  (cl:format cl:nil "# to certain group trooper (immediate answer)~%uint8[] to_uavs~%uint8 from_uav~%~%uint8 allowed_num~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupHelperRes)))
  "Returns full string definition for message of type 'GroupHelperRes"
  (cl:format cl:nil "# to certain group trooper (immediate answer)~%uint8[] to_uavs~%uint8 from_uav~%~%uint8 allowed_num~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupHelperRes>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupHelperRes>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupHelperRes
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':allowed_num (allowed_num msg))
))
