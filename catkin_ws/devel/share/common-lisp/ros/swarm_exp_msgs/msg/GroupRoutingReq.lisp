; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupRoutingReq.msg.html

(cl:defclass <GroupRoutingReq> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GroupRoutingReq (<GroupRoutingReq>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupRoutingReq>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupRoutingReq)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupRoutingReq> is deprecated: use swarm_exp_msgs-msg:GroupRoutingReq instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupRoutingReq>) ostream)
  "Serializes a message object of type '<GroupRoutingReq>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupRoutingReq>) istream)
  "Deserializes a message object of type '<GroupRoutingReq>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupRoutingReq>)))
  "Returns string type for a message object of type '<GroupRoutingReq>"
  "swarm_exp_msgs/GroupRoutingReq")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupRoutingReq)))
  "Returns string type for a message object of type 'GroupRoutingReq"
  "swarm_exp_msgs/GroupRoutingReq")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupRoutingReq>)))
  "Returns md5sum for a message object of type '<GroupRoutingReq>"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupRoutingReq)))
  "Returns md5sum for a message object of type 'GroupRoutingReq"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupRoutingReq>)))
  "Returns full string definition for message of type '<GroupRoutingReq>"
  (cl:format cl:nil "# inside group, from trooper (Event Timer)~%~%# problem formation and answer space~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupRoutingReq)))
  "Returns full string definition for message of type 'GroupRoutingReq"
  (cl:format cl:nil "# inside group, from trooper (Event Timer)~%~%# problem formation and answer space~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupRoutingReq>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupRoutingReq>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupRoutingReq
))
