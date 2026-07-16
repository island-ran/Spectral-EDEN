; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgFNode.msg.html

(cl:defclass <DtgFNode> (roslisp-msg-protocol:ros-message)
  ((f_id
    :reader f_id
    :initarg :f_id
    :type cl:fixnum
    :initform 0)
   (alive
    :reader alive
    :initarg :alive
    :type cl:boolean
    :initform cl:nil)
   (vp_flags
    :reader vp_flags
    :initarg :vp_flags
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (need_help
    :reader need_help
    :initarg :need_help
    :type cl:fixnum
    :initform 0))
)

(cl:defclass DtgFNode (<DtgFNode>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgFNode>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgFNode)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgFNode> is deprecated: use swarm_exp_msgs-msg:DtgFNode instead.")))

(cl:ensure-generic-function 'f_id-val :lambda-list '(m))
(cl:defmethod f_id-val ((m <DtgFNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:f_id-val is deprecated.  Use swarm_exp_msgs-msg:f_id instead.")
  (f_id m))

(cl:ensure-generic-function 'alive-val :lambda-list '(m))
(cl:defmethod alive-val ((m <DtgFNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:alive-val is deprecated.  Use swarm_exp_msgs-msg:alive instead.")
  (alive m))

(cl:ensure-generic-function 'vp_flags-val :lambda-list '(m))
(cl:defmethod vp_flags-val ((m <DtgFNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:vp_flags-val is deprecated.  Use swarm_exp_msgs-msg:vp_flags instead.")
  (vp_flags m))

(cl:ensure-generic-function 'need_help-val :lambda-list '(m))
(cl:defmethod need_help-val ((m <DtgFNode>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:need_help-val is deprecated.  Use swarm_exp_msgs-msg:need_help instead.")
  (need_help m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgFNode>) ostream)
  "Serializes a message object of type '<DtgFNode>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'alive) 1 0)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'vp_flags))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'vp_flags))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'need_help)) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgFNode>) istream)
  "Deserializes a message object of type '<DtgFNode>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'f_id)) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'alive) (cl:not (cl:zerop (cl:read-byte istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'vp_flags) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'vp_flags)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:aref vals i)) (cl:read-byte istream)))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'need_help)) (cl:read-byte istream))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgFNode>)))
  "Returns string type for a message object of type '<DtgFNode>"
  "swarm_exp_msgs/DtgFNode")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgFNode)))
  "Returns string type for a message object of type 'DtgFNode"
  "swarm_exp_msgs/DtgFNode")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgFNode>)))
  "Returns md5sum for a message object of type '<DtgFNode>"
  "f35023aa28359878229c76466f958b4c")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgFNode)))
  "Returns md5sum for a message object of type 'DtgFNode"
  "f35023aa28359878229c76466f958b4c")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgFNode>)))
  "Returns full string definition for message of type '<DtgFNode>"
  (cl:format cl:nil "uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgFNode)))
  "Returns full string definition for message of type 'DtgFNode"
  (cl:format cl:nil "uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgFNode>))
  (cl:+ 0
     2
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'vp_flags) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     1
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgFNode>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgFNode
    (cl:cons ':f_id (f_id msg))
    (cl:cons ':alive (alive msg))
    (cl:cons ':vp_flags (vp_flags msg))
    (cl:cons ':need_help (need_help msg))
))
