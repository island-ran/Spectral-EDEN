; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupHelperReq.msg.html

(cl:defclass <GroupHelperReq> (roslisp-msg-protocol:ros-message)
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
   (helper_ids
    :reader helper_ids
    :initarg :helper_ids
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (cost
    :reader cost
    :initarg :cost
    :type cl:float
    :initform 0.0))
)

(cl:defclass GroupHelperReq (<GroupHelperReq>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupHelperReq>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupHelperReq)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupHelperReq> is deprecated: use swarm_exp_msgs-msg:GroupHelperReq instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <GroupHelperReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <GroupHelperReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'helper_ids-val :lambda-list '(m))
(cl:defmethod helper_ids-val ((m <GroupHelperReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:helper_ids-val is deprecated.  Use swarm_exp_msgs-msg:helper_ids instead.")
  (helper_ids m))

(cl:ensure-generic-function 'cost-val :lambda-list '(m))
(cl:defmethod cost-val ((m <GroupHelperReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:cost-val is deprecated.  Use swarm_exp_msgs-msg:cost instead.")
  (cost m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupHelperReq>) ostream)
  "Serializes a message object of type '<GroupHelperReq>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'helper_ids))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'helper_ids))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'cost))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupHelperReq>) istream)
  "Deserializes a message object of type '<GroupHelperReq>"
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
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'helper_ids) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'helper_ids)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'cost) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupHelperReq>)))
  "Returns string type for a message object of type '<GroupHelperReq>"
  "swarm_exp_msgs/GroupHelperReq")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupHelperReq)))
  "Returns string type for a message object of type 'GroupHelperReq"
  "swarm_exp_msgs/GroupHelperReq")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupHelperReq>)))
  "Returns md5sum for a message object of type '<GroupHelperReq>"
  "de1dd505e44758dd2e830a3b48f4bffe")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupHelperReq)))
  "Returns md5sum for a message object of type 'GroupHelperReq"
  "de1dd505e44758dd2e830a3b48f4bffe")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupHelperReq>)))
  "Returns full string definition for message of type '<GroupHelperReq>"
  (cl:format cl:nil "# to certain group trooper (immediate answer)~%uint8[] to_uavs~%uint8 from_uav~%~%uint8[] helper_ids~%float32 cost~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupHelperReq)))
  "Returns full string definition for message of type 'GroupHelperReq"
  (cl:format cl:nil "# to certain group trooper (immediate answer)~%uint8[] to_uavs~%uint8 from_uav~%~%uint8[] helper_ids~%float32 cost~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupHelperReq>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'helper_ids) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupHelperReq>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupHelperReq
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':helper_ids (helper_ids msg))
    (cl:cons ':cost (cost msg))
))
