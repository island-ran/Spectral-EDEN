; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgHNode.msg.html

(cl:defclass <DtgHNode> (roslisp-msg-protocol:ros-message)
  ((h_id
    :reader h_id
    :initarg :h_id
    :type cl:integer
    :initform 0)
   (pos_idx
    :reader pos_idx
    :initarg :pos_idx
    :type cl:integer
    :initform 0))
)

(cl:defclass DtgHNode (<DtgHNode>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgHNode>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgHNode)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgHNode> is deprecated: use swarm_exp_msgs-msg:DtgHNode instead.")))

(cl:ensure-generic-function 'h_id-val :lambda-list '(m))
(cl:defmethod h_id-val ((m <DtgHNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:h_id-val is deprecated.  Use swarm_exp_msgs-msg:h_id instead.")
  (h_id m))

(cl:ensure-generic-function 'pos_idx-val :lambda-list '(m))
(cl:defmethod pos_idx-val ((m <DtgHNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:pos_idx-val is deprecated.  Use swarm_exp_msgs-msg:pos_idx instead.")
  (pos_idx m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgHNode>) ostream)
  "Serializes a message object of type '<DtgHNode>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'h_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'pos_idx)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'pos_idx)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'pos_idx)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'pos_idx)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgHNode>) istream)
  "Deserializes a message object of type '<DtgHNode>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'h_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'pos_idx)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'pos_idx)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'pos_idx)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'pos_idx)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgHNode>)))
  "Returns string type for a message object of type '<DtgHNode>"
  "swarm_exp_msgs/DtgHNode")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgHNode)))
  "Returns string type for a message object of type 'DtgHNode"
  "swarm_exp_msgs/DtgHNode")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgHNode>)))
  "Returns md5sum for a message object of type '<DtgHNode>"
  "61b258874ecfcb669e1516b9e9ebe709")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgHNode)))
  "Returns md5sum for a message object of type 'DtgHNode"
  "61b258874ecfcb669e1516b9e9ebe709")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgHNode>)))
  "Returns full string definition for message of type '<DtgHNode>"
  (cl:format cl:nil "uint32 h_id~%uint32 pos_idx #low res map~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgHNode)))
  "Returns full string definition for message of type 'DtgHNode"
  (cl:format cl:nil "uint32 h_id~%uint32 pos_idx #low res map~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgHNode>))
  (cl:+ 0
     4
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgHNode>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgHNode
    (cl:cons ':h_id (h_id msg))
    (cl:cons ':pos_idx (pos_idx msg))
))
