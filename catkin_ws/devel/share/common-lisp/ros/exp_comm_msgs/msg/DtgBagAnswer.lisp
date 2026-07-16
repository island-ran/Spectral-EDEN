; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude DtgBagAnswer.msg.html

(cl:defclass <DtgBagAnswer> (roslisp-msg-protocol:ros-message)
  ((id
    :reader id
    :initarg :id
    :type cl:fixnum
    :initform 0)
   (to_uav
    :reader to_uav
    :initarg :to_uav
    :type cl:fixnum
    :initform 0)
   (bag_id
    :reader bag_id
    :initarg :bag_id
    :type cl:integer
    :initform 0))
)

(cl:defclass DtgBagAnswer (<DtgBagAnswer>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgBagAnswer>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgBagAnswer)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<DtgBagAnswer> is deprecated: use exp_comm_msgs-msg:DtgBagAnswer instead.")))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <DtgBagAnswer>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:id-val is deprecated.  Use exp_comm_msgs-msg:id instead.")
  (id m))

(cl:ensure-generic-function 'to_uav-val :lambda-list '(m))
(cl:defmethod to_uav-val ((m <DtgBagAnswer>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:to_uav-val is deprecated.  Use exp_comm_msgs-msg:to_uav instead.")
  (to_uav m))

(cl:ensure-generic-function 'bag_id-val :lambda-list '(m))
(cl:defmethod bag_id-val ((m <DtgBagAnswer>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:bag_id-val is deprecated.  Use exp_comm_msgs-msg:bag_id instead.")
  (bag_id m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgBagAnswer>) ostream)
  "Serializes a message object of type '<DtgBagAnswer>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'to_uav)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'bag_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'bag_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'bag_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'bag_id)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgBagAnswer>) istream)
  "Deserializes a message object of type '<DtgBagAnswer>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'to_uav)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'bag_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'bag_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'bag_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'bag_id)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgBagAnswer>)))
  "Returns string type for a message object of type '<DtgBagAnswer>"
  "exp_comm_msgs/DtgBagAnswer")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgBagAnswer)))
  "Returns string type for a message object of type 'DtgBagAnswer"
  "exp_comm_msgs/DtgBagAnswer")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgBagAnswer>)))
  "Returns md5sum for a message object of type '<DtgBagAnswer>"
  "38ef4e8bfd92de589cce314b18d25dfb")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgBagAnswer)))
  "Returns md5sum for a message object of type 'DtgBagAnswer"
  "38ef4e8bfd92de589cce314b18d25dfb")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgBagAnswer>)))
  "Returns full string definition for message of type '<DtgBagAnswer>"
  (cl:format cl:nil "uint8 id~%uint8 to_uav~%uint32 bag_id~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgBagAnswer)))
  "Returns full string definition for message of type 'DtgBagAnswer"
  (cl:format cl:nil "uint8 id~%uint8 to_uav~%uint32 bag_id~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgBagAnswer>))
  (cl:+ 0
     1
     1
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgBagAnswer>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgBagAnswer
    (cl:cons ':id (id msg))
    (cl:cons ':to_uav (to_uav msg))
    (cl:cons ':bag_id (bag_id msg))
))
