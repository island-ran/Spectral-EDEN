; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupCommandReq.msg.html

(cl:defclass <GroupCommandReq> (roslisp-msg-protocol:ros-message)
  ((to_uavs
    :reader to_uavs
    :initarg :to_uavs
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (cur_t
    :reader cur_t
    :initarg :cur_t
    :type cl:float
    :initform 0.0)
   (dispatch_type
    :reader dispatch_type
    :initarg :dispatch_type
    :type cl:fixnum
    :initform 0)
   (departure_EROIs
    :reader departure_EROIs
    :initarg :departure_EROIs
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (departure_status
    :reader departure_status
    :initarg :departure_status
    :type cl:fixnum
    :initform 0)
   (helped_group_id
    :reader helped_group_id
    :initarg :helped_group_id
    :type cl:fixnum
    :initform 0))
)

(cl:defclass GroupCommandReq (<GroupCommandReq>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupCommandReq>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupCommandReq)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupCommandReq> is deprecated: use swarm_exp_msgs-msg:GroupCommandReq instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'cur_t-val :lambda-list '(m))
(cl:defmethod cur_t-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:cur_t-val is deprecated.  Use swarm_exp_msgs-msg:cur_t instead.")
  (cur_t m))

(cl:ensure-generic-function 'dispatch_type-val :lambda-list '(m))
(cl:defmethod dispatch_type-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:dispatch_type-val is deprecated.  Use swarm_exp_msgs-msg:dispatch_type instead.")
  (dispatch_type m))

(cl:ensure-generic-function 'departure_EROIs-val :lambda-list '(m))
(cl:defmethod departure_EROIs-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:departure_EROIs-val is deprecated.  Use swarm_exp_msgs-msg:departure_EROIs instead.")
  (departure_EROIs m))

(cl:ensure-generic-function 'departure_status-val :lambda-list '(m))
(cl:defmethod departure_status-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:departure_status-val is deprecated.  Use swarm_exp_msgs-msg:departure_status instead.")
  (departure_status m))

(cl:ensure-generic-function 'helped_group_id-val :lambda-list '(m))
(cl:defmethod helped_group_id-val ((m <GroupCommandReq>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:helped_group_id-val is deprecated.  Use swarm_exp_msgs-msg:helped_group_id instead.")
  (helped_group_id m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupCommandReq>) ostream)
  "Serializes a message object of type '<GroupCommandReq>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'cur_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'dispatch_type)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'departure_EROIs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) ele) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) ele) ostream))
   (cl:slot-value msg 'departure_EROIs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'departure_status)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'helped_group_id)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupCommandReq>) istream)
  "Deserializes a message object of type '<GroupCommandReq>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'to_uavs) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'to_uavs)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'dispatch_type)) (cl:read-byte istream))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'departure_EROIs) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'departure_EROIs)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:aref vals i)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'departure_status)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'helped_group_id)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupCommandReq>)))
  "Returns string type for a message object of type '<GroupCommandReq>"
  "swarm_exp_msgs/GroupCommandReq")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupCommandReq)))
  "Returns string type for a message object of type 'GroupCommandReq"
  "swarm_exp_msgs/GroupCommandReq")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupCommandReq>)))
  "Returns md5sum for a message object of type '<GroupCommandReq>"
  "8907e16fca4b37e09a5750aaba75e3cc")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupCommandReq)))
  "Returns md5sum for a message object of type 'GroupCommandReq"
  "8907e16fca4b37e09a5750aaba75e3cc")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupCommandReq>)))
  "Returns full string definition for message of type '<GroupCommandReq>"
  (cl:format cl:nil "# inside group, from trooper (Quick timmer)~%uint8[] to_uavs~%# uint8 from_uav~%~%float64 cur_t~%# 1: Departure; 2: Help, 3: switch trooper~%uint8 dispatch_type ~%~%# departure~%uint32[] departure_EROIs~%uint8 departure_status #0: trooper; 1: infantry~%~%#help~%uint8 helped_group_id~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupCommandReq)))
  "Returns full string definition for message of type 'GroupCommandReq"
  (cl:format cl:nil "# inside group, from trooper (Quick timmer)~%uint8[] to_uavs~%# uint8 from_uav~%~%float64 cur_t~%# 1: Departure; 2: Help, 3: switch trooper~%uint8 dispatch_type ~%~%# departure~%uint32[] departure_EROIs~%uint8 departure_status #0: trooper; 1: infantry~%~%#help~%uint8 helped_group_id~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupCommandReq>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     8
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'departure_EROIs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     1
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupCommandReq>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupCommandReq
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':cur_t (cur_t msg))
    (cl:cons ':dispatch_type (dispatch_type msg))
    (cl:cons ':departure_EROIs (departure_EROIs msg))
    (cl:cons ':departure_status (departure_status msg))
    (cl:cons ':helped_group_id (helped_group_id msg))
))
