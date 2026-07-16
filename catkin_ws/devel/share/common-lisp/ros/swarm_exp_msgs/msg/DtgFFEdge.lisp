; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgFFEdge.msg.html

(cl:defclass <DtgFFEdge> (roslisp-msg-protocol:ros-message)
  ((head_f_id
    :reader head_f_id
    :initarg :head_f_id
    :type cl:fixnum
    :initform 0)
   (tail_f_id
    :reader tail_f_id
    :initarg :tail_f_id
    :type cl:fixnum
    :initform 0)
   (points_idx
    :reader points_idx
    :initarg :points_idx
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (head_v_id
    :reader head_v_id
    :initarg :head_v_id
    :type cl:fixnum
    :initform 0)
   (tail_v_id
    :reader tail_v_id
    :initarg :tail_v_id
    :type cl:fixnum
    :initform 0)
   (ffe_id
    :reader ffe_id
    :initarg :ffe_id
    :type cl:fixnum
    :initform 0))
)

(cl:defclass DtgFFEdge (<DtgFFEdge>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgFFEdge>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgFFEdge)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgFFEdge> is deprecated: use swarm_exp_msgs-msg:DtgFFEdge instead.")))

(cl:ensure-generic-function 'head_f_id-val :lambda-list '(m))
(cl:defmethod head_f_id-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:head_f_id-val is deprecated.  Use swarm_exp_msgs-msg:head_f_id instead.")
  (head_f_id m))

(cl:ensure-generic-function 'tail_f_id-val :lambda-list '(m))
(cl:defmethod tail_f_id-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:tail_f_id-val is deprecated.  Use swarm_exp_msgs-msg:tail_f_id instead.")
  (tail_f_id m))

(cl:ensure-generic-function 'points_idx-val :lambda-list '(m))
(cl:defmethod points_idx-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:points_idx-val is deprecated.  Use swarm_exp_msgs-msg:points_idx instead.")
  (points_idx m))

(cl:ensure-generic-function 'head_v_id-val :lambda-list '(m))
(cl:defmethod head_v_id-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:head_v_id-val is deprecated.  Use swarm_exp_msgs-msg:head_v_id instead.")
  (head_v_id m))

(cl:ensure-generic-function 'tail_v_id-val :lambda-list '(m))
(cl:defmethod tail_v_id-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:tail_v_id-val is deprecated.  Use swarm_exp_msgs-msg:tail_v_id instead.")
  (tail_v_id m))

(cl:ensure-generic-function 'ffe_id-val :lambda-list '(m))
(cl:defmethod ffe_id-val ((m <DtgFFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:ffe_id-val is deprecated.  Use swarm_exp_msgs-msg:ffe_id instead.")
  (ffe_id m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgFFEdge>) ostream)
  "Serializes a message object of type '<DtgFFEdge>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'head_f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'tail_f_id)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'points_idx))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) ele) ostream))
   (cl:slot-value msg 'points_idx))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_v_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_v_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ffe_id)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgFFEdge>) istream)
  "Deserializes a message object of type '<DtgFFEdge>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'head_f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'tail_f_id)) (cl:read-byte istream))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'points_idx) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'points_idx)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_v_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_v_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'ffe_id)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgFFEdge>)))
  "Returns string type for a message object of type '<DtgFFEdge>"
  "swarm_exp_msgs/DtgFFEdge")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgFFEdge)))
  "Returns string type for a message object of type 'DtgFFEdge"
  "swarm_exp_msgs/DtgFFEdge")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgFFEdge>)))
  "Returns md5sum for a message object of type '<DtgFFEdge>"
  "764ef8eb22d76355c9c291bdaa1ae595")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgFFEdge)))
  "Returns md5sum for a message object of type 'DtgFFEdge"
  "764ef8eb22d76355c9c291bdaa1ae595")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgFFEdge>)))
  "Returns full string definition for message of type '<DtgFFEdge>"
  (cl:format cl:nil "uint16 head_f_id~%uint16 tail_f_id~%uint32[] points_idx #low res map~%~%uint8 head_v_id #viewpoint id of head fn~%uint8 tail_v_id #viewpoint id of tail fn~%uint8 ffe_id    #who creates~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgFFEdge)))
  "Returns full string definition for message of type 'DtgFFEdge"
  (cl:format cl:nil "uint16 head_f_id~%uint16 tail_f_id~%uint32[] points_idx #low res map~%~%uint8 head_v_id #viewpoint id of head fn~%uint8 tail_v_id #viewpoint id of tail fn~%uint8 ffe_id    #who creates~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgFFEdge>))
  (cl:+ 0
     2
     2
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'points_idx) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     1
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgFFEdge>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgFFEdge
    (cl:cons ':head_f_id (head_f_id msg))
    (cl:cons ':tail_f_id (tail_f_id msg))
    (cl:cons ':points_idx (points_idx msg))
    (cl:cons ':head_v_id (head_v_id msg))
    (cl:cons ':tail_v_id (tail_v_id msg))
    (cl:cons ':ffe_id (ffe_id msg))
))
