; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgHFEdge.msg.html

(cl:defclass <DtgHFEdge> (roslisp-msg-protocol:ros-message)
  ((points_idx
    :reader points_idx
    :initarg :points_idx
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
   (f_id
    :reader f_id
    :initarg :f_id
    :type cl:fixnum
    :initform 0)
   (h_id
    :reader h_id
    :initarg :h_id
    :type cl:integer
    :initform 0)
   (vp_id
    :reader vp_id
    :initarg :vp_id
    :type cl:fixnum
    :initform 0))
)

(cl:defclass DtgHFEdge (<DtgHFEdge>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgHFEdge>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgHFEdge)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgHFEdge> is deprecated: use swarm_exp_msgs-msg:DtgHFEdge instead.")))

(cl:ensure-generic-function 'points_idx-val :lambda-list '(m))
(cl:defmethod points_idx-val ((m <DtgHFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:points_idx-val is deprecated.  Use swarm_exp_msgs-msg:points_idx instead.")
  (points_idx m))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <DtgHFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:pub_t-val is deprecated.  Use swarm_exp_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'f_id-val :lambda-list '(m))
(cl:defmethod f_id-val ((m <DtgHFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:f_id-val is deprecated.  Use swarm_exp_msgs-msg:f_id instead.")
  (f_id m))

(cl:ensure-generic-function 'h_id-val :lambda-list '(m))
(cl:defmethod h_id-val ((m <DtgHFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:h_id-val is deprecated.  Use swarm_exp_msgs-msg:h_id instead.")
  (h_id m))

(cl:ensure-generic-function 'vp_id-val :lambda-list '(m))
(cl:defmethod vp_id-val ((m <DtgHFEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:vp_id-val is deprecated.  Use swarm_exp_msgs-msg:vp_id instead.")
  (vp_id m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgHFEdge>) ostream)
  "Serializes a message object of type '<DtgHFEdge>"
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
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vp_id)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgHFEdge>) istream)
  "Deserializes a message object of type '<DtgHFEdge>"
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'vp_id)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgHFEdge>)))
  "Returns string type for a message object of type '<DtgHFEdge>"
  "swarm_exp_msgs/DtgHFEdge")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgHFEdge)))
  "Returns string type for a message object of type 'DtgHFEdge"
  "swarm_exp_msgs/DtgHFEdge")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgHFEdge>)))
  "Returns md5sum for a message object of type '<DtgHFEdge>"
  "e9f0be3eb0a7ce0d70ee65a834e21ddf")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgHFEdge)))
  "Returns md5sum for a message object of type 'DtgHFEdge"
  "e9f0be3eb0a7ce0d70ee65a834e21ddf")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgHFEdge>)))
  "Returns full string definition for message of type '<DtgHFEdge>"
  (cl:format cl:nil "uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgHFEdge)))
  "Returns full string definition for message of type 'DtgHFEdge"
  (cl:format cl:nil "uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgHFEdge>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'points_idx) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     8
     2
     4
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgHFEdge>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgHFEdge
    (cl:cons ':points_idx (points_idx msg))
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':f_id (f_id msg))
    (cl:cons ':h_id (h_id msg))
    (cl:cons ':vp_id (vp_id msg))
))
