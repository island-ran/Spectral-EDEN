; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupState.msg.html

(cl:defclass <GroupState> (roslisp-msg-protocol:ros-message)
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
   (group_size
    :reader group_size
    :initarg :group_size
    :type cl:fixnum
    :initform 0)
   (trooper_targets
    :reader trooper_targets
    :initarg :trooper_targets
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (potential
    :reader potential
    :initarg :potential
    :type cl:fixnum
    :initform 0)
   (coverage_work_load
    :reader coverage_work_load
    :initarg :coverage_work_load
    :type cl:fixnum
    :initform 0)
   (state
    :reader state
    :initarg :state
    :type cl:fixnum
    :initform 0))
)

(cl:defclass GroupState (<GroupState>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupState>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupState)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupState> is deprecated: use swarm_exp_msgs-msg:GroupState instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'cur_t-val :lambda-list '(m))
(cl:defmethod cur_t-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:cur_t-val is deprecated.  Use swarm_exp_msgs-msg:cur_t instead.")
  (cur_t m))

(cl:ensure-generic-function 'group_size-val :lambda-list '(m))
(cl:defmethod group_size-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:group_size-val is deprecated.  Use swarm_exp_msgs-msg:group_size instead.")
  (group_size m))

(cl:ensure-generic-function 'trooper_targets-val :lambda-list '(m))
(cl:defmethod trooper_targets-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:trooper_targets-val is deprecated.  Use swarm_exp_msgs-msg:trooper_targets instead.")
  (trooper_targets m))

(cl:ensure-generic-function 'potential-val :lambda-list '(m))
(cl:defmethod potential-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:potential-val is deprecated.  Use swarm_exp_msgs-msg:potential instead.")
  (potential m))

(cl:ensure-generic-function 'coverage_work_load-val :lambda-list '(m))
(cl:defmethod coverage_work_load-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:coverage_work_load-val is deprecated.  Use swarm_exp_msgs-msg:coverage_work_load instead.")
  (coverage_work_load m))

(cl:ensure-generic-function 'state-val :lambda-list '(m))
(cl:defmethod state-val ((m <GroupState>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:state-val is deprecated.  Use swarm_exp_msgs-msg:state instead.")
  (state m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupState>) ostream)
  "Serializes a message object of type '<GroupState>"
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
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_size)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'trooper_targets))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) ele) ostream))
   (cl:slot-value msg 'trooper_targets))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'potential)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'potential)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'coverage_work_load)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'coverage_work_load)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'state)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupState>) istream)
  "Deserializes a message object of type '<GroupState>"
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'group_size)) (cl:read-byte istream))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'trooper_targets) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'trooper_targets)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'potential)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'potential)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'coverage_work_load)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'coverage_work_load)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'state)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupState>)))
  "Returns string type for a message object of type '<GroupState>"
  "swarm_exp_msgs/GroupState")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupState)))
  "Returns string type for a message object of type 'GroupState"
  "swarm_exp_msgs/GroupState")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupState>)))
  "Returns md5sum for a message object of type '<GroupState>"
  "17f63ea80422701593a5c33c707b7615")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupState)))
  "Returns md5sum for a message object of type 'GroupState"
  "17f63ea80422701593a5c33c707b7615")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupState>)))
  "Returns full string definition for message of type '<GroupState>"
  (cl:format cl:nil "# inter group broad cast (slow timer)~%uint8[] to_uavs~%uint8 from_uav~%~%float64 cur_t~%uint8 group_size~%uint32[] trooper_targets~%uint16 potential #calculated unknown space number, for helpers~%uint16 coverage_work_load~%uint8 state # 0000 00(need help)(exp/cover)~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupState)))
  "Returns full string definition for message of type 'GroupState"
  (cl:format cl:nil "# inter group broad cast (slow timer)~%uint8[] to_uavs~%uint8 from_uav~%~%float64 cur_t~%uint8 group_size~%uint32[] trooper_targets~%uint16 potential #calculated unknown space number, for helpers~%uint16 coverage_work_load~%uint8 state # 0000 00(need help)(exp/cover)~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupState>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     8
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'trooper_targets) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     2
     2
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupState>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupState
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':cur_t (cur_t msg))
    (cl:cons ':group_size (group_size msg))
    (cl:cons ':trooper_targets (trooper_targets msg))
    (cl:cons ':potential (potential msg))
    (cl:cons ':coverage_work_load (coverage_work_load msg))
    (cl:cons ':state (state msg))
))
