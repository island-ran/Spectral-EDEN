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

class DtgHFEdge {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.points_idx = null;
      this.pub_t = null;
      this.f_id = null;
      this.h_id = null;
      this.vp_id = null;
    }
    else {
      if (initObj.hasOwnProperty('points_idx')) {
        this.points_idx = initObj.points_idx
      }
      else {
        this.points_idx = [];
      }
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('f_id')) {
        this.f_id = initObj.f_id
      }
      else {
        this.f_id = 0;
      }
      if (initObj.hasOwnProperty('h_id')) {
        this.h_id = initObj.h_id
      }
      else {
        this.h_id = 0;
      }
      if (initObj.hasOwnProperty('vp_id')) {
        this.vp_id = initObj.vp_id
      }
      else {
        this.vp_id = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgHFEdge
    // Serialize message field [points_idx]
    bufferOffset = _arraySerializer.uint32(obj.points_idx, buffer, bufferOffset, null);
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [f_id]
    bufferOffset = _serializer.uint16(obj.f_id, buffer, bufferOffset);
    // Serialize message field [h_id]
    bufferOffset = _serializer.uint32(obj.h_id, buffer, bufferOffset);
    // Serialize message field [vp_id]
    bufferOffset = _serializer.uint8(obj.vp_id, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgHFEdge
    let len;
    let data = new DtgHFEdge(null);
    // Deserialize message field [points_idx]
    data.points_idx = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [f_id]
    data.f_id = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [h_id]
    data.h_id = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [vp_id]
    data.vp_id = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 4 * object.points_idx.length;
    return length + 19;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgHFEdge';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'e9f0be3eb0a7ce0d70ee65a834e21ddf';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint32[] points_idx #low res map
    float64 pub_t
    uint16 f_id
    uint32 h_id
    uint8 vp_id
    #uint8 hfe_id        #who creates
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgHFEdge(null);
    if (msg.points_idx !== undefined) {
      resolved.points_idx = msg.points_idx;
    }
    else {
      resolved.points_idx = []
    }

    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.f_id !== undefined) {
      resolved.f_id = msg.f_id;
    }
    else {
      resolved.f_id = 0
    }

    if (msg.h_id !== undefined) {
      resolved.h_id = msg.h_id;
    }
    else {
      resolved.h_id = 0
    }

    if (msg.vp_id !== undefined) {
      resolved.vp_id = msg.vp_id;
    }
    else {
      resolved.vp_id = 0
    }

    return resolved;
    }
};

module.exports = DtgHFEdge;
