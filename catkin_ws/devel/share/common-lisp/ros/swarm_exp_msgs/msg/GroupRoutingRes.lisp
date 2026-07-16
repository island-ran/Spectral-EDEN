; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupRoutingRes.msg.html

(cl:defclass <GroupRoutingRes> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GroupRoutingRes (<GroupRoutingRes>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupRoutingRes>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupRoutingRes)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupRoutingRes> is deprecated: use swarm_exp_msgs-msg:GroupRoutingRes instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupRoutingRes>) ostream)
  "Serializes a message object of type '<GroupRoutingRes>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupRoutingRes>) istream)
  "Deserializes a message object of type '<GroupRoutingRes>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupRoutingRes>)))
  "Returns string type for a message object of type '<GroupRoutingRes>"
  "swarm_exp_msgs/GroupRoutingRes")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupRoutingRes)))
  "Returns string type for a message object of type 'GroupRoutingRes"
  "swarm_exp_msgs/GroupRoutingRes")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupRoutingRes>)))
  "Returns md5sum for a message object of type '<GroupRoutingRes>"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupRoutingRes)))
  "Returns md5sum for a message object of type 'GroupRoutingRes"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupRoutingRes>)))
  "Returns full string definition for message of type '<GroupRoutingRes>"
  (cl:format cl:nil "# inside group, to trooper (Immediate answer)~%~%# best answer~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupRoutingRes)))
  "Returns full string definition for message of type 'GroupRoutingRes"
  (cl:format cl:nil "# inside group, to trooper (Immediate answer)~%~%# best answer~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupRoutingRes>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupRoutingRes>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupRoutingRes
))
