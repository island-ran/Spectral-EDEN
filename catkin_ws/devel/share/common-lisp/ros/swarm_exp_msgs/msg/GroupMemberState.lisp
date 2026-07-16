; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupMemberState.msg.html

(cl:defclass <GroupMemberState> (roslisp-msg-protocol:ros-message)
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
   (cur_t
    :reader cur_t
    :initarg :cur_t
    :type cl:float
    :initform 0.0)
   (targets
    :reader targets
    :initarg :targets
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (work_type
    :reader work_type
    :initarg :work_type
    :type cl:fixnum
    :initform 0))
)

(cl:defclass GroupMemberState (<GroupMemberState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupMemberState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupMemberState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupMemberState> is deprecated: use swarm_exp_msgs-msg:GroupMemberState instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <GroupMemberState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <GroupMemberState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'cur_t-val :lambda-list '(m))
(cl:defmethod cur_t-val ((m <GroupMemberState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:cur_t-val is deprecated.  Use swarm_exp_msgs-msg:cur_t instead.")
  (cur_t m))

(cl:ensure-generic-function 'targets-val :lambda-list '(m))
(cl:defmethod targets-val ((m <GroupMemberState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:targets-val is deprecated.  Use swarm_exp_msgs-msg:targets instead.")
  (targets m))

(cl:ensure-generic-function 'work_type-val :lambda-list '(m))
(cl:defmethod work_type-val ((m <GroupMemberState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:work_type-val is deprecated.  Use swarm_exp_msgs-msg:work_type instead.")
  (work_type m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupMemberState>) ostream)
  "Serializes a message object of type '<GroupMemberState>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'cur_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'targets))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) ele) ostream))
   (cl:slot-value msg 'targets))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'work_type)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupMemberState>) istream)
  "Deserializes a message object of type '<GroupMemberState>"
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
    (cl:setf (cl:slot-value msg 'cur_t) (roslisp-utils:decode-double-float-bits bits)))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'targets) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'targets)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'work_type)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupMemberState>)))
  "Returns string type for a message object of type '<GroupMemberState>"
  "swarm_exp_msgs/GroupMemberState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupMemberState)))
  "Returns string type for a message object of type 'GroupMemberState"
  "swarm_exp_msgs/GroupMemberState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupMemberState>)))
  "Returns md5sum for a message object of type '<GroupMemberState>"
  "abb6338d4d1acda965b2e312dd0c2bb4")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupMemberState)))
  "Returns md5sum for a message object of type 'GroupMemberState"
  "abb6338d4d1acda965b2e312dd0c2bb4")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupMemberState>)))
  "Returns full string definition for message of type '<GroupMemberState>"
  (cl:format cl:nil "# group broad cast (quick timer)~%uint8[] to_uavs~%uint8 from_uav~%~%float64 cur_t~%uint32[] targets~%uint8 work_type #cover, explore, follow, wait~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupMemberState)))
  "Returns full string definition for message of type 'GroupMemberState"
  (cl:format cl:nil "# group broad cast (quick timer)~%uint8[] to_uavs~%uint8 from_uav~%~%float64 cur_t~%uint32[] targets~%uint8 work_type #cover, explore, follow, wait~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupMemberState>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     8
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'targets) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupMemberState>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupMemberState
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':cur_t (cur_t msg))
    (cl:cons ':targets (targets msg))
    (cl:cons ':work_type (work_type msg))
))
