// Auto-generated. Do not edit!

// (in-package exp_comm_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class MapReqC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.f_id = null;
      this.block_id = null;
      this.flag = null;
    }
    else {
      if (initObj.hasOwnProperty('f_id')) {
        this.f_id = initObj.f_id
      }
      else {
        this.f_id = [];
      }
      if (initObj.hasOwnProperty('block_id')) {
        this.block_id = initObj.block_id
      }
      else {
        this.block_id = [];
      }
      if (initObj.hasOwnProperty('flag')) {
        this.flag = initObj.flag
      }
      else {
        this.flag = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type MapReqC
    // Serialize message field [f_id]
    bufferOffset = _arraySerializer.uint16(obj.f_id, buffer, bufferOffset, null);
    // Serialize message field [block_id]
    bufferOffset = _arraySerializer.uint8(obj.block_id, buffer, bufferOffset, null);
    // Serialize message field [flag]
    bufferOffset = _serializer.uint8(obj.flag, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type MapReqC
    let len;
    let data = new MapReqC(null);
    // Deserialize message field [f_id]
    data.f_id = _arrayDeserializer.uint16(buffer, bufferOffset, null)
    // Deserialize message field [block_id]
    data.block_id = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [flag]
    data.flag = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 2 * object.f_id.length;
    length += object.block_id.length;
    return length + 9;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/MapReqC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '71af631c49d7e61ef9c70b1b03b18a05';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint16[] f_id
    uint8[] block_id
    uint8 flag
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new MapReqC(null);
    if (msg.f_id !== undefined) {
      resolved.f_id = msg.f_id;
    }
    else {
      resolved.f_id = []
    }

    if (msg.block_id !== undefined) {
      resolved.block_id = msg.block_id;
    }
    else {
      resolved.block_id = []
    }

    if (msg.flag !== undefined) {
      resolved.flag = msg.flag;
    }
    else {
      resolved.flag = 0
    }

    return resolved;
    }
};

module.exports = MapReqC;
