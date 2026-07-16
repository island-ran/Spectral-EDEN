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

class DtgHNode {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.h_id = null;
      this.pos_idx = null;
    }
    else {
      if (initObj.hasOwnProperty('h_id')) {
        this.h_id = initObj.h_id
      }
      else {
        this.h_id = 0;
      }
      if (initObj.hasOwnProperty('pos_idx')) {
        this.pos_idx = initObj.pos_idx
      }
      else {
        this.pos_idx = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgHNode
    // Serialize message field [h_id]
    bufferOffset = _serializer.uint32(obj.h_id, buffer, bufferOffset);
    // Serialize message field [pos_idx]
    bufferOffset = _serializer.uint32(obj.pos_idx, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgHNode
    let len;
    let data = new DtgHNode(null);
    // Deserialize message field [h_id]
    data.h_id = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [pos_idx]
    data.pos_idx = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgHNode';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '61b258874ecfcb669e1516b9e9ebe709';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint32 h_id
    uint32 pos_idx #low res map
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgHNode(null);
    if (msg.h_id !== undefined) {
      resolved.h_id = msg.h_id;
    }
    else {
      resolved.h_id = 0
    }

    if (msg.pos_idx !== undefined) {
      resolved.pos_idx = msg.pos_idx;
    }
    else {
      resolved.pos_idx = 0
    }

    return resolved;
    }
};

module.exports = DtgHNode;
