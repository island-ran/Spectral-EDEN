// Auto-generated. Do not edit!

// (in-package swarm_exp_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let DtgHFEdge = require('./DtgHFEdge.js');
let DtgHHEdge = require('./DtgHHEdge.js');
let DtgHNode = require('./DtgHNode.js');
let DtgFNode = require('./DtgFNode.js');

//-----------------------------------------------------------

class DtgBag {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.id = null;
      this.from_uav = null;
      this.HFedges = null;
      this.HHedges = null;
      this.Hnodes = null;
      this.Fnodes = null;
    }
    else {
      if (initObj.hasOwnProperty('to_uavs')) {
        this.to_uavs = initObj.to_uavs
      }
      else {
        this.to_uavs = [];
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
    // Serializes a message object of type DtgBag
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [id]
    bufferOffset = _serializer.uint32(obj.id, buffer, bufferOffset);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [HFedges]
    // Serialize the length for message field [HFedges]
    bufferOffset = _serializer.uint32(obj.HFedges.length, buffer, bufferOffset);
    obj.HFedges.forEach((val) => {
      bufferOffset = DtgHFEdge.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [HHedges]
    // Serialize the length for message field [HHedges]
    bufferOffset = _serializer.uint32(obj.HHedges.length, buffer, bufferOffset);
    obj.HHedges.forEach((val) => {
      bufferOffset = DtgHHEdge.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [Hnodes]
    // Serialize the length for message field [Hnodes]
    bufferOffset = _serializer.uint32(obj.Hnodes.length, buffer, bufferOffset);
    obj.Hnodes.forEach((val) => {
      bufferOffset = DtgHNode.serialize(val, buffer, bufferOffset);
    });
    // Serialize message field [Fnodes]
    // Serialize the length for message field [Fnodes]
    bufferOffset = _serializer.uint32(obj.Fnodes.length, buffer, bufferOffset);
    obj.Fnodes.forEach((val) => {
      bufferOffset = DtgFNode.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgBag
    let len;
    let data = new DtgBag(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [id]
    data.id = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [HFedges]
    // Deserialize array length for message field [HFedges]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.HFedges = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.HFedges[i] = DtgHFEdge.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [HHedges]
    // Deserialize array length for message field [HHedges]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.HHedges = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.HHedges[i] = DtgHHEdge.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [Hnodes]
    // Deserialize array length for message field [Hnodes]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.Hnodes = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.Hnodes[i] = DtgHNode.deserialize(buffer, bufferOffset)
    }
    // Deserialize message field [Fnodes]
    // Deserialize array length for message field [Fnodes]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.Fnodes = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.Fnodes[i] = DtgFNode.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    object.HFedges.forEach((val) => {
      length += DtgHFEdge.getMessageSize(val);
    });
    object.HHedges.forEach((val) => {
      length += DtgHHEdge.getMessageSize(val);
    });
    length += 8 * object.Hnodes.length;
    object.Fnodes.forEach((val) => {
      length += DtgFNode.getMessageSize(val);
    });
    return length + 25;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgBag';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '0fad25fdf73ad19fd3ef0fed38c94965';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # groupe broad cast (quick timer for local or group / slow timer for general)
    
    uint8[] to_uavs
    uint32 id
    uint8 from_uav
    
    swarm_exp_msgs/DtgHFEdge[] HFedges
    swarm_exp_msgs/DtgHHEdge[] HHedges
    swarm_exp_msgs/DtgHNode[] Hnodes
    swarm_exp_msgs/DtgFNode[] Fnodes 
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
    const resolved = new DtgBag(null);
    if (msg.to_uavs !== undefined) {
      resolved.to_uavs = msg.to_uavs;
    }
    else {
      resolved.to_uavs = []
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

    if (msg.HFedges !== undefined) {
      resolved.HFedges = new Array(msg.HFedges.length);
      for (let i = 0; i < resolved.HFedges.length; ++i) {
        resolved.HFedges[i] = DtgHFEdge.Resolve(msg.HFedges[i]);
      }
    }
    else {
      resolved.HFedges = []
    }

    if (msg.HHedges !== undefined) {
      resolved.HHedges = new Array(msg.HHedges.length);
      for (let i = 0; i < resolved.HHedges.length; ++i) {
        resolved.HHedges[i] = DtgHHEdge.Resolve(msg.HHedges[i]);
      }
    }
    else {
      resolved.HHedges = []
    }

    if (msg.Hnodes !== undefined) {
      resolved.Hnodes = new Array(msg.Hnodes.length);
      for (let i = 0; i < resolved.Hnodes.length; ++i) {
        resolved.Hnodes[i] = DtgHNode.Resolve(msg.Hnodes[i]);
      }
    }
    else {
      resolved.Hnodes = []
    }

    if (msg.Fnodes !== undefined) {
      resolved.Fnodes = new Array(msg.Fnodes.length);
      for (let i = 0; i < resolved.Fnodes.length; ++i) {
        resolved.Fnodes[i] = DtgFNode.Resolve(msg.Fnodes[i]);
      }
    }
    else {
      resolved.Fnodes = []
    }

    return resolved;
    }
};

module.exports = DtgBag;
