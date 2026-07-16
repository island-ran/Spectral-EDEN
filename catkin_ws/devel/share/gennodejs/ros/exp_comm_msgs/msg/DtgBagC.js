// Auto-generated. Do not edit!

// (in-package exp_comm_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let swarm_exp_msgs = _finder('swarm_exp_msgs');

//-----------------------------------------------------------

class DtgBagC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.pub_t = null;
      this.id = null;
      this.from_uav = null;
      this.FFedges = null;
      this.HFedges = null;
      this.HHedges = null;
      this.Hnodes = null;
      this.Fnodes = null;
    }
    else {
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('id')) {
        this.id = initObj.id
      }
      else {
        this.id = 0;
      }
      if (initObj.hasOwnProperty('from_uav')) {
        this.from_uav = initObj.from_uav
      }
      else {
        this.from_uav = 0;
      }
      if (initObj.hasOwnProperty('FFedges')) {
        this.FFedges = initObj.FFedges
      }
      else {
        this.FFedges = [];
      }
      if (initObj.hasOwnProperty('HFedges')) {
        this.HFedges = initObj.HFedges
      }
      else {
        this.HFedges = [];
      }
      if (initObj.hasOwnProperty('HHedges')) {
        this.HHedges = initObj.HHedges
      }
      else {
        this.HHedges = [];
      }
      if (initObj.hasOwnProperty('Hnodes')) {
        this.Hnodes = initObj.Hnodes
      }
      else {
        this.Hnodes = [];
      }
      if (initObj.hasOwnProperty('Fnodes')) {
        this.Fnodes = initObj.Fnodes
      }
      else {
        this.Fnodes = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgBagC
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [id]
    bufferOffset = _serializer.uint32(obj.id, buffer, bufferOffset);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [FFedges]
    // Serialize the length for message field [FFedges]
    bufferOffset = _serializer.uint32(obj.FFedges.length, buffer, bufferOffset);
    obj.FFedges.forEach((val) => {
      bufferOffset = swarm_exp_msgs.msg.DtgFFEdge.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [HFedges]
    // Serialize the length for message field [HFedges]
    bufferOffset = _serializer.uint32(obj.HFedges.length, buffer, bufferOffset);
    obj.HFedges.forEach((val) => {
      bufferOffset = swarm_exp_msgs.msg.DtgHFEdge.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [HHedges]
    // Serialize the length for message field [HHedges]
    bufferOffset = _serializer.uint32(obj.HHedges.length, buffer, bufferOffset);
    obj.HHedges.forEach((val) => {
      bufferOffset = swarm_exp_msgs.msg.DtgHHEdge.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [Hnodes]
    // Serialize the length for message field [Hnodes]
    bufferOffset = _serializer.uint32(obj.Hnodes.length, buffer, bufferOffset);
    obj.Hnodes.forEach((val) => {
      bufferOffset = swarm_exp_msgs.msg.DtgHNode.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [Fnodes]
    // Serialize the length for message field [Fnodes]
    bufferOffset = _serializer.uint32(obj.Fnodes.length, buffer, bufferOffset);
    obj.Fnodes.forEach((val) => {
      bufferOffset = swarm_exp_msgs.msg.DtgFNode.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgBagC
    let len;
    let data = new DtgBagC(null);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [id]
    data.id = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [FFedges]
    // Deserialize array length for message field [FFedges]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.FFedges = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.FFedges[i] = swarm_exp_msgs.msg.DtgFFEdge.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [HFedges]
    // Deserialize array length for message field [HFedges]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.HFedges = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.HFedges[i] = swarm_exp_msgs.msg.DtgHFEdge.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [HHedges]
    // Deserialize array length for message field [HHedges]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.HHedges = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.HHedges[i] = swarm_exp_msgs.msg.DtgHHEdge.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [Hnodes]
    // Deserialize array length for message field [Hnodes]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.Hnodes = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.Hnodes[i] = swarm_exp_msgs.msg.DtgHNode.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [Fnodes]
    // Deserialize array length for message field [Fnodes]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.Fnodes = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.Fnodes[i] = swarm_exp_msgs.msg.DtgFNode.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    object.FFedges.forEach((val) => {
      length += swarm_exp_msgs.msg.DtgFFEdge.getMessageSize(val);
    });
    object.HFedges.forEach((val) => {
      length += swarm_exp_msgs.msg.DtgHFEdge.getMessageSize(val);
    });
    object.HHedges.forEach((val) => {
      length += swarm_exp_msgs.msg.DtgHHEdge.getMessageSize(val);
    });
    length += 8 * object.Hnodes.length;
    object.Fnodes.forEach((val) => {
      length += swarm_exp_msgs.msg.DtgFNode.getMessageSize(val);
    });
    return length + 33;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/DtgBagC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ff41106c2b664f3ededcb2a753d90eeb';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 pub_t
    uint32 id
    uint8 from_uav
    swarm_exp_msgs/DtgFFEdge[] FFedges
    swarm_exp_msgs/DtgHFEdge[] HFedges
    swarm_exp_msgs/DtgHHEdge[] HHedges
    swarm_exp_msgs/DtgHNode[] Hnodes
    swarm_exp_msgs/DtgFNode[] Fnodes 
    ================================================================================
    MSG: swarm_exp_msgs/DtgFFEdge
    uint16 head_f_id
    uint16 tail_f_id
    uint32[] points_idx #low res map
    
    uint8 head_v_id #viewpoint id of head fn
    uint8 tail_v_id #viewpoint id of tail fn
    uint8 ffe_id    #who creates
    
    ================================================================================
    MSG: swarm_exp_msgs/DtgHFEdge
    uint32[] points_idx #low res map
    float64 pub_t
    uint16 f_id
    uint32 h_id
    uint8 vp_id
    #uint8 hfe_id        #who creates
    
    ================================================================================
    MSG: swarm_exp_msgs/DtgHHEdge
    #uint8 hhe_id        #who creates
    #bool erase 
    float64 pub_t
    uint32[] points_idx #low res map
    uint32 head_h_id
    uint32 tail_h_id
    ================================================================================
    MSG: swarm_exp_msgs/DtgHNode
    uint32 h_id
    uint32 pos_idx #low res map
    ================================================================================
    MSG: swarm_exp_msgs/DtgFNode
    uint16 f_id
    bool alive
    uint8[] vp_flags #(alive dead)(alive dead)(alive dead)(alive dead)
    uint8 need_help # ask other uav for edge
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgBagC(null);
    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.id !== undefined) {
      resolved.id = msg.id;
    }
    else {
      resolved.id = 0
    }

    if (msg.from_uav !== undefined) {
      resolved.from_uav = msg.from_uav;
    }
    else {
      resolved.from_uav = 0
    }

    if (msg.FFedges !== undefined) {
      resolved.FFedges = new Array(msg.FFedges.length);
      for (let i = 0; i < resolved.FFedges.length; ++i) {
        resolved.FFedges[i] = swarm_exp_msgs.msg.DtgFFEdge.Resolve(msg.FFedges[i]);
      }
    }
    else {
      resolved.FFedges = []
    }

    if (msg.HFedges !== undefined) {
      resolved.HFedges = new Array(msg.HFedges.length);
      for (let i = 0; i < resolved.HFedges.length; ++i) {
        resolved.HFedges[i] = swarm_exp_msgs.msg.DtgHFEdge.Resolve(msg.HFedges[i]);
      }
    }
    else {
      resolved.HFedges = []
    }

    if (msg.HHedges !== undefined) {
      resolved.HHedges = new Array(msg.HHedges.length);
      for (let i = 0; i < resolved.HHedges.length; ++i) {
        resolved.HHedges[i] = swarm_exp_msgs.msg.DtgHHEdge.Resolve(msg.HHedges[i]);
      }
    }
    else {
      resolved.HHedges = []
    }

    if (msg.Hnodes !== undefined) {
      resolved.Hnodes = new Array(msg.Hnodes.length);
      for (let i = 0; i < resolved.Hnodes.length; ++i) {
        resolved.Hnodes[i] = swarm_exp_msgs.msg.DtgHNode.Resolve(msg.Hnodes[i]);
      }
    }
    else {
      resolved.Hnodes = []
    }

    if (msg.Fnodes !== undefined) {
      resolved.Fnodes = new Array(msg.Fnodes.length);
      for (let i = 0; i < resolved.Fnodes.length; ++i) {
        resolved.Fnodes[i] = swarm_exp_msgs.msg.DtgFNode.Resolve(msg.Fnodes[i]);
      }
    }
    else {
      resolved.Fnodes = []
    }

    return resolved;
    }
};

module.exports = DtgBagC;
