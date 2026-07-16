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

class DtgHHEdge {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.pub_t = null;
      this.points_idx = null;
      this.head_h_id = null;
      this.tail_h_id = null;
    }
    else {
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('points_idx')) {
        this.points_idx = initObj.points_idx
      }
      else {
        this.points_idx = [];
      }
      if (initObj.hasOwnProperty('head_h_id')) {
        this.head_h_id = initObj.head_h_id
      }
      else {
        this.head_h_id = 0;
      }
      if (initObj.hasOwnProperty('tail_h_id')) {
        this.tail_h_id = initObj.tail_h_id
      }
      else {
        this.tail_h_id = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgHHEdge
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [points_idx]
    bufferOffset = _arraySerializer.uint32(obj.points_idx, buffer, bufferOffset, null);
    // Serialize message field [head_h_id]
    bufferOffset = _serializer.uint32(obj.head_h_id, buffer, bufferOffset);
    // Serialize message field [tail_h_id]
    bufferOffset = _serializer.uint32(obj.tail_h_id, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgHHEdge
    let len;
    let data = new DtgHHEdge(null);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [points_idx]
    data.points_idx = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [head_h_id]
    data.head_h_id = _deserializer.uint32(buffer, bufferOffset);
    // Deserialize message field [tail_h_id]
    data.tail_h_id = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 4 * object.points_idx.length;
    return length + 20;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgHHEdge';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f28c77b458771baddef5547b4e2a8c2d';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    #uint8 hhe_id        #who creates
    #bool erase 
    float64 pub_t
    uint32[] points_idx #low res map
    uint32 head_h_id
    uint32 tail_h_id
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgHHEdge(null);
    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.points_idx !== undefined) {
      resolved.points_idx = msg.points_idx;
    }
    else {
      resolved.points_idx = []
    }

    if (msg.head_h_id !== undefined) {
      resolved.head_h_id = msg.head_h_id;
    }
    else {
      resolved.head_h_id = 0
    }

    if (msg.tail_h_id !== undefined) {
      resolved.tail_h_id = msg.tail_h_id;
    }
    else {
      resolved.tail_h_id = 0
    }

    return resolved;
    }
};

module.exports = DtgHHEdge;
