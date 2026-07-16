; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupCommandRes.msg.html

(cl:defclass <GroupCommandRes> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GroupCommandRes (<GroupCommandRes>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupCommandRes>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupCommandRes)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupCommandRes> is deprecated: use swarm_exp_msgs-msg:GroupCommandRes instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupCommandRes>) ostream)
  "Serializes a message object of type '<GroupCommandRes>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupCommandRes>) istream)
  "Deserializes a message object of type '<GroupCommandRes>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupCommandRes>)))
  "Returns string type for a message object of type '<GroupCommandRes>"
  "swarm_exp_msgs/GroupCommandRes")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupCommandRes)))
  "Returns string type for a message object of type 'GroupCommandRes"
  "swarm_exp_msgs/GroupCommandRes")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupCommandRes>)))
  "Returns md5sum for a message object of type '<GroupCommandRes>"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupCommandRes)))
  "Returns md5sum for a message object of type 'GroupCommandRes"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupCommandRes>)))
  "Returns full string definition for message of type '<GroupCommandRes>"
  (cl:format cl:nil "# inside group, to trooper (Immediate answer)~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupCommandRes)))
  "Returns full string definition for message of type 'GroupCommandRes"
  (cl:format cl:nil "# inside group, to trooper (Immediate answer)~%~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupCommandRes>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupCommandRes>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupCommandRes
))
