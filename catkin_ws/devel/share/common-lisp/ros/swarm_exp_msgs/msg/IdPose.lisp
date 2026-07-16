; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude IdPose.msg.html

(cl:defclass <IdPose> (roslisp-msg-protocol:ros-message)
  ((to_uavs
    :reader to_uavs
    :initarg :to_uavs
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (id
    :reader id
    :initarg :id
    :type cl:fixnum
    :initform 0)
   (pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
   (pose
    :reader pose
    :initarg :pose
    :type geometry_msgs-msg:Pose
    :initform (cl:make-instance 'geometry_msgs-msg:Pose)))
)

(cl:defclass IdPose (<IdPose>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <IdPose>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'IdPose)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<IdPose> is deprecated: use swarm_exp_msgs-msg:IdPose instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <IdPose>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <IdPose>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:id-val is deprecated.  Use swarm_exp_msgs-msg:id instead.")
  (id m))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <IdPose>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:pub_t-val is deprecated.  Use swarm_exp_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'pose-val :lambda-list '(m))
(cl:defmethod pose-val ((m <IdPose>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:pose-val is deprecated.  Use swarm_exp_msgs-msg:pose instead.")
  (pose m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <IdPose>) ostream)
  "Serializes a message object of type '<IdPose>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) ostream)
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'pose) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <IdPose>) istream)
  "Deserializes a message object of type '<IdPose>"
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'to_uavs) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'to_uavs)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) (cl:read-byte istream))
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
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'pose) istream)
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<IdPose>)))
  "Returns string type for a message object of type '<IdPose>"
  "swarm_exp_msgs/IdPose")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'IdPose)))
  "Returns string type for a message object of type 'IdPose"
  "swarm_exp_msgs/IdPose")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<IdPose>)))
  "Returns md5sum for a message object of type '<IdPose>"
  "cc5f67458fe878e0f625ae446543e38e")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'IdPose)))
  "Returns md5sum for a message object of type 'IdPose"
  "cc5f67458fe878e0f625ae446543e38e")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<IdPose>)))
  "Returns full string definition for message of type '<IdPose>"
  (cl:format cl:nil "# immediate for local or group, slow timer for general~%uint8[] to_uavs~%~%uint8 id~%float64 pub_t~%geometry_msgs/Pose pose~%================================================================================~%MSG: geometry_msgs/Pose~%# A representation of pose in free space, composed of position and orientation. ~%Point position~%Quaternion orientation~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%================================================================================~%MSG: geometry_msgs/Quaternion~%# This represents an orientation in free space in quaternion form.~%~%float64 x~%float64 y~%float64 z~%float64 w~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'IdPose)))
  "Returns full string definition for message of type 'IdPose"
  (cl:format cl:nil "# immediate for local or group, slow timer for general~%uint8[] to_uavs~%~%uint8 id~%float64 pub_t~%geometry_msgs/Pose pose~%================================================================================~%MSG: geometry_msgs/Pose~%# A representation of pose in free space, composed of position and orientation. ~%Point position~%Quaternion orientation~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%================================================================================~%MSG: geometry_msgs/Quaternion~%# This represents an orientation in free space in quaternion form.~%~%float64 x~%float64 y~%float64 z~%float64 w~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <IdPose>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
     8
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'pose))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <IdPose>))
  "Converts a ROS message object to a list"
  (cl:list 'IdPose
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':id (id msg))
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':pose (pose msg))
))
