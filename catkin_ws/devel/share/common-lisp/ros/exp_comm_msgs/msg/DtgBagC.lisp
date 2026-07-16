; Auto-generated. Do not edit!


(cl:in-package exp_comm_msgs-msg)


;//! \htmlinclude DtgBagC.msg.html

(cl:defclass <DtgBagC> (roslisp-msg-protocol:ros-message)
  ((pub_t
    :reader pub_t
    :initarg :pub_t
    :type cl:float
    :initform 0.0)
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
   (FFedges
    :reader FFedges
    :initarg :FFedges
    :type (cl:vector swarm_exp_msgs-msg:DtgFFEdge)
   :initform (cl:make-array 0 :element-type 'swarm_exp_msgs-msg:DtgFFEdge :initial-element (cl:make-instance 'swarm_exp_msgs-msg:DtgFFEdge)))
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

(cl:defclass DtgBagC (<DtgBagC>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <DtgBagC>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'DtgBagC)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name exp_comm_msgs-msg:<DtgBagC> is deprecated: use exp_comm_msgs-msg:DtgBagC instead.")))

(cl:ensure-generic-function 'pub_t-val :lambda-list '(m))
(cl:defmethod pub_t-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:pub_t-val is deprecated.  Use exp_comm_msgs-msg:pub_t instead.")
  (pub_t m))

(cl:ensure-generic-function 'id-val :lambda-list '(m))
(cl:defmethod id-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:id-val is deprecated.  Use exp_comm_msgs-msg:id instead.")
  (id m))

(cl:ensure-generic-function 'from_uav-val :lambda-list '(m))
(cl:defmethod from_uav-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:from_uav-val is deprecated.  Use exp_comm_msgs-msg:from_uav instead.")
  (from_uav m))

(cl:ensure-generic-function 'FFedges-val :lambda-list '(m))
(cl:defmethod FFedges-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:FFedges-val is deprecated.  Use exp_comm_msgs-msg:FFedges instead.")
  (FFedges m))

(cl:ensure-generic-function 'HFedges-val :lambda-list '(m))
(cl:defmethod HFedges-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:HFedges-val is deprecated.  Use exp_comm_msgs-msg:HFedges instead.")
  (HFedges m))

(cl:ensure-generic-function 'HHedges-val :lambda-list '(m))
(cl:defmethod HHedges-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:HHedges-val is deprecated.  Use exp_comm_msgs-msg:HHedges instead.")
  (HHedges m))

(cl:ensure-generic-function 'Hnodes-val :lambda-list '(m))
(cl:defmethod Hnodes-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:Hnodes-val is deprecated.  Use exp_comm_msgs-msg:Hnodes instead.")
  (Hnodes m))

(cl:ensure-generic-function 'Fnodes-val :lambda-list '(m))
(cl:defmethod Fnodes-val ((m <DtgBagC>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader exp_comm_msgs-msg:Fnodes-val is deprecated.  Use exp_comm_msgs-msg:Fnodes instead.")
  (Fnodes m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <DtgBagC>) ostream)
  "Serializes a message object of type '<DtgBagC>"
  (cl:let ((bits (roslisp-utils:encode-double-float-bits (cl:slot-value msg 'pub_t))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 32) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 40) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 48) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 56) bits) ostream))
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 8) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 16) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 24) (cl:slot-value msg 'id)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'from_uav)) ostream)
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'FFedges))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (roslisp-msg-protocol:serialize ele ostream))
   (cl:slot-value msg 'FFedges))
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
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <DtgBagC>) istream)
  "Deserializes a message object of type '<DtgBagC>"
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
  (cl:setf (cl:slot-value msg 'FFedges) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'FFedges)))
    (cl:dotimes (i __ros_arr_len)
    (cl:setf (cl:aref vals i) (cl:make-instance 'swarm_exp_msgs-msg:DtgFFEdge))
  (roslisp-msg-protocol:deserialize (cl:aref vals i) istream))))
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
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<DtgBagC>)))
  "Returns string type for a message object of type '<DtgBagC>"
  "exp_comm_msgs/DtgBagC")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'DtgBagC)))
  "Returns string type for a message object of type 'DtgBagC"
  "exp_comm_msgs/DtgBagC")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<DtgBagC>)))
  "Returns md5sum for a message object of type '<DtgBagC>"
  "ff41106c2b664f3ededcb2a753d90eeb")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'DtgBagC)))
  "Returns md5sum for a message object of type 'DtgBagC"
  "ff41106c2b664f3ededcb2a753d90eeb")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<DtgBagC>)))
  "Returns full string definition for message of type '<DtgBagC>"
  (cl:format cl:nil "float64 pub_t~%uint32 id~%uint8 from_uav~%swarm_exp_msgs/DtgFFEdge[] FFedges~%swarm_exp_msgs/DtgHFEdge[] HFedges~%swarm_exp_msgs/DtgHHEdge[] HHedges~%swarm_exp_msgs/DtgHNode[] Hnodes~%swarm_exp_msgs/DtgFNode[] Fnodes ~%================================================================================~%MSG: swarm_exp_msgs/DtgFFEdge~%uint16 head_f_id~%uint16 tail_f_id~%uint32[] points_idx #low res map~%~%uint8 head_v_id #viewpoint id of head fn~%uint8 tail_v_id #viewpoint id of tail fn~%uint8 ffe_id    #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHFEdge~%uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHHEdge~%#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%================================================================================~%MSG: swarm_exp_msgs/DtgHNode~%uint32 h_id~%uint32 pos_idx #low res map~%================================================================================~%MSG: swarm_exp_msgs/DtgFNode~%uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'DtgBagC)))
  "Returns full string definition for message of type 'DtgBagC"
  (cl:format cl:nil "float64 pub_t~%uint32 id~%uint8 from_uav~%swarm_exp_msgs/DtgFFEdge[] FFedges~%swarm_exp_msgs/DtgHFEdge[] HFedges~%swarm_exp_msgs/DtgHHEdge[] HHedges~%swarm_exp_msgs/DtgHNode[] Hnodes~%swarm_exp_msgs/DtgFNode[] Fnodes ~%================================================================================~%MSG: swarm_exp_msgs/DtgFFEdge~%uint16 head_f_id~%uint16 tail_f_id~%uint32[] points_idx #low res map~%~%uint8 head_v_id #viewpoint id of head fn~%uint8 tail_v_id #viewpoint id of tail fn~%uint8 ffe_id    #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHFEdge~%uint32[] points_idx #low res map~%float64 pub_t~%uint16 f_id~%uint32 h_id~%uint8 vp_id~%#uint8 hfe_id        #who creates~%~%================================================================================~%MSG: swarm_exp_msgs/DtgHHEdge~%#uint8 hhe_id        #who creates~%#bool erase ~%float64 pub_t~%uint32[] points_idx #low res map~%uint32 head_h_id~%uint32 tail_h_id~%================================================================================~%MSG: swarm_exp_msgs/DtgHNode~%uint32 h_id~%uint32 pos_idx #low res map~%================================================================================~%MSG: swarm_exp_msgs/DtgFNode~%uint16 f_id~%bool alive~%uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)~%uint8 need_help # ask other uav for edge~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <DtgBagC>))
  (cl:+ 0
     8
     4
     1
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'FFedges) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'HFedges) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'HHedges) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'Hnodes) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'Fnodes) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ (roslisp-msg-protocol:serialization-length ele))))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <DtgBagC>))
  "Converts a ROS message object to a list"
  (cl:list 'DtgBagC
    (cl:cons ':pub_t (pub_t msg))
    (cl:cons ':id (id msg))
    (cl:cons ':from_uav (from_uav msg))
    (cl:cons ':FFedges (FFedges msg))
    (cl:cons ':HFedges (HFedges msg))
    (cl:cons ':HHedges (HHedges msg))
    (cl:cons ':Hnodes (Hnodes msg))
    (cl:cons ':Fnodes (Fnodes msg))
))
