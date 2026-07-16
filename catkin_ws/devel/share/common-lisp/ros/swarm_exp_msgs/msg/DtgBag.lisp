; Auto-generated. Do not edit!


(cl:in-package swarm_exp_msgs-msg)


;//! \htmlinclude DtgBag.msg.html

(cl:defclass <DtgBag> (roslisp-msg-protocol:ros-message)
  ((to_uavs
    :reader to_uavs
    :initarg :to_uavs
    :type (cl:vector cl:fixnum)
   :initform (cl:make-array 0 :element-type 'cl:fixnum :initial-element 0))
   (id
    :reader id
    :initarg :id
    :type cl:integer
    :initform 0)
   (from_uav
    :reader from_uav
    :initarg :from_uav
    :type cl:fixnum
    :initform 0)
   (HFedges
    :reader HFedges
    :initarg :HFedges
    :type (cl:vector swarm_exp_msgs-msg:DtgHFEdge)
   :initform (cl:make-array 0 :element-type 'swarm_exp_msgs-msg:DtgHFEdge :initial-element (cl:make-instance 'swarm_exp_msgs-msg:DtgHFEdge)))
   (HHedges
    :reader HHedges
    :initarg :HHedges
    :type (cl:vector swarm_exp_msgs-msg:DtgHHEdge)
   :initform (cl:make-array 0 :element-type 'swarm_exp_msgs-msg:DtgHHEdge :initial-element (cl:make-instance 'swarm_exp_msgs-msg:DtgHHEdge)))
   (Hnodes
    :reader Hnodes
    :initarg :Hnodes
    :type (cl:vector swarm_exp_msgs-msg:DtgHNode)
   :initform (cl:make-array 0 :element-type 'swarm_exp_msgs-msg:DtgHNode :initial-element (cl:make-instance 'swarm_exp_msgs-msg:DtgHNode)))
   (Fnodes
    :reader Fnodes
    :initarg :Fnodes
    :type (cl:vector swarm_exp_msgs-msg:DtgFNode)
   :initform (cl:make-array 0 :element-type 'swarm_exp_msgs-msg:DtgFNode :initial-element (cl:make-instance 'swarm_exp_msgs-msg:DtgFNode))))
)

(cl:defclass DtgBag (<DtgBag>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgBag>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgBag)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name swarm_exp_msgs-msg:<DtgBag> is deprecated: use swarm_exp_msgs-msg:DtgBag instead.")))

(cl:ensure-generic-function 'to_uavs-val :lambda-list '(m))
(cl:defmethod to_uavs-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:to_uavs-val is deprecated.  Use swarm_exp_msgs-msg:to_uavs instead.")
  (to_uavs m))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:id-val is deprecated.  Use swarm_exp_msgs-msg:id instead.")
  (id m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:from_uav-val is deprecated.  Use swarm_exp_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'HFedges-val :lambda-list '(m))
(cl:defmethod HFedges-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:HFedges-val is deprecated.  Use swarm_exp_msgs-msg:HFedges instead.")
  (HFedges m))

(cl:ensure-generic-function 'HHedges-val :lambda-list '(m))
(cl:defmethod HHedges-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:HHedges-val is deprecated.  Use swarm_exp_msgs-msg:HHedges instead.")
  (HHedges m))

(cl:ensure-generic-function 'Hnodes-val :lambda-list '(m))
(cl:defmethod Hnodes-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:Hnodes-val is deprecated.  Use swarm_exp_msgs-msg:Hnodes instead.")
  (Hnodes m))

(cl:ensure-generic-function 'Fnodes-val :lambda-list '(m))
(cl:defmethod Fnodes-val ((m <DtgBag>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader swarm_exp_msgs-msg:Fnodes-val is deprecated.  Use swarm_exp_msgs-msg:Fnodes instead.")
  (Fnodes m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgBag>) ostream)
  "Serializes a message object of type '<DtgBag>"
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'to_uavs))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:write-byte (cl:ldb (cl:byte 8 0) ele) ostream))
   (cl:slot-value msg 'to_uavs))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'HFedges))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'HFedges))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'HHedges))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'HHedges))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'Hnodes))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'Hnodes))
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'Fnodes))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'Fnodes))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgBag>) istream)
  "Deserializes a message object of type '<DtgBag>"
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
    (cl:setf (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'id)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) (cl:read-byte istream))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'HFedges) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'HFedges)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'swarm_exp_msgs-msg:DtgHFEdge))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'HHedges) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'HHedges)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'swarm_exp_msgs-msg:DtgHHEdge))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'Hnodes) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'Hnodes)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'swarm_exp_msgs-msg:DtgHNode))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'Fnodes) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'Fnodes)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'swarm_exp_msgs-msg:DtgFNode))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgBag>)))
  "Returns string type for a message object of type '<DtgBag>"
  "swarm_exp_msgs/DtgBag")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgBag)))
  "Returns string type for a message object of type 'DtgBag"
  "swarm_exp_msgs/DtgBag")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgBag>)))
  "Returns md5sum for a message object of type '<DtgBag>"
  "0fad25fdf73ad19fd3ef0fed38c94965")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgBag)))
  "Returns md5sum for a message object of type 'DtgBag"
  "0fad25fdf73ad19fd3ef0fed38c94965")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgBag>)))
  "Returns full string definition for message of type '<DtgBag>"
  (cl:format cl:nil "# groupe broad cast (quick timer for local or group / slow timer for general)~%~%uint8[] to_uavs~%uint32 id~%uint8 from_uav~%~%swarm_exp_msgs/DtgHFEdge[] HFedges~%swarm_exp_msgs/DtgHHEdge[] HHedges~%swarm_exp_msgs/DtgHNode[] Hnodes~%swarm_exp_msgs/DtgFNode[] Fnodes ~%================================================================================~%MSG: swarm_exp_msgs/DtgHFEdge~%uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHHEdge~%#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%================================================================================~%MSG: swarm_exp_msgs/DtgHNode~%uint32 h_id~%uint32 pos_idx #low res map~%================================================================================~%MSG: swarm_exp_msgs/DtgFNode~%uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgBag)))
  "Returns full string definition for message of type 'DtgBag"
  (cl:format cl:nil "# groupe broad cast (quick timer for local or group / slow timer for general)~%~%uint8[] to_uavs~%uint32 id~%uint8 from_uav~%~%swarm_exp_msgs/DtgHFEdge[] HFedges~%swarm_exp_msgs/DtgHHEdge[] HHedges~%swarm_exp_msgs/DtgHNode[] Hnodes~%swarm_exp_msgs/DtgFNode[] Fnodes ~%================================================================================~%MSG: swarm_exp_msgs/DtgHFEdge~%uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHHEdge~%#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%================================================================================~%MSG: swarm_exp_msgs/DtgHNode~%uint32 h_id~%uint32 pos_idx #low res map~%================================================================================~%MSG: swarm_exp_msgs/DtgFNode~%uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgBag>))
  (cl:+ 0
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'to_uavs) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 1)))
     4
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'HFedges) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'HHedges) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'Hnodes) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'Fnodes) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgBag>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgBag
    (cl:cons ':to_uavs (to_uavs msg))
    (cl:cons ':id (id msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':HFedges (HFedges msg))
    (cl:cons ':HHedges (HHedges msg))
    (cl:cons ':Hnodes (Hnodes msg))
    (cl:cons ':Fnodes (Fnodes msg))
))
