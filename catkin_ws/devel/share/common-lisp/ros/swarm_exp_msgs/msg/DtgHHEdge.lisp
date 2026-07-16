; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgHHEdge.msg.html

(cl:defclass <DtgHHEdge> (roslisp-msg-protocol:ros-message)
  ((pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
   (points_idx
    :reader points_idx
    :initarg :points_idx
    :type (cl:vector cl:integer)
   :initform (cl:make-array 0 :element-type 'cl:integer :initial-element 0))
   (head_h_id
    :reader head_h_id
    :initarg :head_h_id
    :type cl:integer
    :initform 0)
   (tail_h_id
    :reader tail_h_id
    :initarg :tail_h_id
    :type cl:integer
    :initform 0))
)

(cl:defclass DtgHHEdge (<DtgHHEdge>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgHHEdge>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgHHEdge)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgHHEdge> is deprecated: use swarm_exp_msgs-msg:DtgHHEdge instead.")))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <DtgHHEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:pub_t-val is deprecated.  Use swarm_exp_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'points_idx-val :lambda-list '(m))
(cl:defmethod points_idx-val ((m <DtgHHEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:points_idx-val is deprecated.  Use swarm_exp_msgs-msg:points_idx instead.")
  (points_idx m))

(cl:ensure-generic-function 'head_h_id-val :lambda-list '(m))
(cl:defmethod head_h_id-val ((m <DtgHHEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:head_h_id-val is deprecated.  Use swarm_exp_msgs-msg:head_h_id instead.")
  (head_h_id m))

(cl:ensure-generic-function 'tail_h_id-val :lambda-list '(m))
(cl:defmethod tail_h_id-val ((m <DtgHHEdge>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:tail_h_id-val is deprecated.  Use swarm_exp_msgs-msg:tail_h_id instead.")
  (tail_h_id m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgHHEdge>) ostream)
  "Serializes a message object of type '<DtgHHEdge>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
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
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'head_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'head_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'head_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'tail_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'tail_h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'tail_h_id)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgHHEdge>) istream)
  "Deserializes a message object of type '<DtgHHEdge>"
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
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'head_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'head_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'head_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'head_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'tail_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'tail_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'tail_h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'tail_h_id)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgHHEdge>)))
  "Returns string type for a message object of type '<DtgHHEdge>"
  "swarm_exp_msgs/DtgHHEdge")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgHHEdge)))
  "Returns string type for a message object of type 'DtgHHEdge"
  "swarm_exp_msgs/DtgHHEdge")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgHHEdge>)))
  "Returns md5sum for a message object of type '<DtgHHEdge>"
  "f28c77b458771baddef5547b4e2a8c2d")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgHHEdge)))
  "Returns md5sum for a message object of type 'DtgHHEdge"
  "f28c77b458771baddef5547b4e2a8c2d")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgHHEdge>)))
  "Returns full string definition for message of type '<DtgHHEdge>"
  (cl:format cl:nil "#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgHHEdge)))
  "Returns full string definition for message of type 'DtgHHEdge"
  (cl:format cl:nil "#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgHHEdge>))
  (cl:+ 0
     8
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'points_idx) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgHHEdge>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgHHEdge
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':points_idx (points_idx msg))
    (cl:cons ':head_h_id (head_h_id msg))
    (cl:cons ':tail_h_id (tail_h_id msg))
))
