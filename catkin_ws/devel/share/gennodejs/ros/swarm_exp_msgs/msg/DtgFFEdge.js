// Auto-generated. Do not edit!

// (in-package swarm_exp_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class DtgFFEdge {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.head_f_id = null;
      this.tail_f_id = null;
      this.points_idx = null;
      this.head_v_id = null;
      this.tail_v_id = null;
      this.ffe_id = null;
    }
    else {
      if (initObj.hasOwnProperty('head_f_id')) {
        this.head_f_id = initObj.head_f_id
      }
      else {
        this.head_f_id = 0;
      }
      if (initObj.hasOwnProperty('tail_f_id')) {
        this.tail_f_id = initObj.tail_f_id
      }
      else {
        this.tail_f_id = 0;
      }
      if (initObj.hasOwnProperty('points_idx')) {
        this.points_idx = initObj.points_idx
      }
      else {
        this.points_idx = [];
      }
      if (initObj.hasOwnProperty('head_v_id')) {
        this.head_v_id = initObj.head_v_id
      }
      else {
        this.head_v_id = 0;
      }
      if (initObj.hasOwnProperty('tail_v_id')) {
        this.tail_v_id = initObj.tail_v_id
      }
      else {
        this.tail_v_id = 0;
      }
      if (initObj.hasOwnProperty('ffe_id')) {
        this.ffe_id = initObj.ffe_id
      }
      else {
        this.ffe_id = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgFFEdge
    // Serialize message field [head_f_id]
    bufferOffset = _serializer.uint16(obj.head_f_id, buffer, bufferOffset);
    // Serialize message field [tail_f_id]
    bufferOffset = _serializer.uint16(obj.tail_f_id, buffer, bufferOffset);
    // Serialize message field [points_idx]
    bufferOffset = _arraySerializer.uint32(obj.points_idx, buffer, bufferOffset, null);
    // Serialize message field [head_v_id]
    bufferOffset = _serializer.uint8(obj.head_v_id, buffer, bufferOffset);
    // Serialize message field [tail_v_id]
    bufferOffset = _serializer.uint8(obj.tail_v_id, buffer, bufferOffset);
    // Serialize message field [ffe_id]
    bufferOffset = _serializer.uint8(obj.ffe_id, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgFFEdge
    let len;
    let data = new DtgFFEdge(null);
    // Deserialize message field [head_f_id]
    data.head_f_id = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [tail_f_id]
    data.tail_f_id = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [points_idx]
    data.points_idx = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [head_v_id]
    data.head_v_id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [tail_v_id]
    data.tail_v_id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [ffe_id]
    data.ffe_id = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 4 * object.points_idx.length;
    return length + 11;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgFFEdge';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '764ef8eb22d76355c9c291bdaa1ae595';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint16 head_f_id
    uint16 tail_f_id
    uint32[] points_idx #low res map
    
    uint8 head_v_id #viewpoint id of head fn
    uint8 tail_v_id #viewpoint id of tail fn
    uint8 ffe_id    #who creates
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgFFEdge(null);
    if (msg.head_f_id !== undefined) {
      resolved.head_f_id = msg.head_f_id;
    }
    else {
      resolved.head_f_id = 0
    }

    if (msg.tail_f_id !== undefined) {
      resolved.tail_f_id = msg.tail_f_id;
    }
    else {
      resolved.tail_f_id = 0
    }

    if (msg.points_idx !== undefined) {
      resolved.points_idx = msg.points_idx;
    }
    else {
      resolved.points_idx = []
    }

    if (msg.head_v_id !== undefined) {
      resolved.head_v_id = msg.head_v_id;
    }
    else {
      resolved.head_v_id = 0
    }

    if (msg.tail_v_id !== undefined) {
      resolved.tail_v_id = msg.tail_v_id;
    }
    else {
      resolved.tail_v_id = 0
    }

    if (msg.ffe_id !== undefined) {
      resolved.ffe_id = msg.ffe_id;
    }
    else {
      resolved.ffe_id = 0
    }

    return resolved;
    }
};

module.exports = DtgFFEdge;
