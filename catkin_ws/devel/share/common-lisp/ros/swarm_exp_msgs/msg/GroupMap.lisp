; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude GroupMap.msg.html

(cl:defclass <GroupMap> (roslisp-msg-protocol:ros-message)
  ()
)

(cl:defclass GroupMap (<GroupMap>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <GroupMap>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'GroupMap)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<GroupMap> is deprecated: use swarm_exp_msgs-msg:GroupMap instead.")))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <GroupMap>) ostream)
  "Serializes a message object of type '<GroupMap>"
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <GroupMap>) istream)
  "Deserializes a message object of type '<GroupMap>"
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<GroupMap>)))
  "Returns string type for a message object of type '<GroupMap>"
  "swarm_exp_msgs/GroupMap")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'GroupMap)))
  "Returns string type for a message object of type 'GroupMap"
  "swarm_exp_msgs/GroupMap")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<GroupMap>)))
  "Returns md5sum for a message object of type '<GroupMap>"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'GroupMap)))
  "Returns md5sum for a message object of type 'GroupMap"
  "d41d8cd98f00b204e9800998ecf8427e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<GroupMap>)))
  "Returns full string definition for message of type '<GroupMap>"
  (cl:format cl:nil "# inside group broad cast (quick timer)~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'GroupMap)))
  "Returns full string definition for message of type 'GroupMap"
  (cl:format cl:nil "# inside group broad cast (quick timer)~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <GroupMap>))
  (cl:+ 0
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <GroupMap>))
  "Converts a ROS message object to a list"
  (cl:list 'GroupMap
))
