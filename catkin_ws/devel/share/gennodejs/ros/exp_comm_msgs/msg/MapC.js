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

class MapC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.f_id = null;
      this.block_id = null;
      this.block_state = null;
      this.flags = null;
    }
    else {
      if (initObj.hasOwnProperty('f_id')) {
        this.f_id = initObj.f_id
      }
      else {
        this.f_id = 0;
      }
      if (initObj.hasOwnProperty('block_id')) {
        this.block_id = initObj.block_id
      }
      else {
        this.block_id = 0;
      }
      if (initObj.hasOwnProperty('block_state')) {
        this.block_state = initObj.block_state
      }
      else {
        this.block_state = 0;
      }
      if (initObj.hasOwnProperty('flags')) {
        this.flags = initObj.flags
      }
      else {
        this.flags = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type MapC
    // Serialize message field [f_id]
    bufferOffset = _serializer.uint16(obj.f_id, buffer, bufferOffset);
    // Serialize message field [block_id]
    bufferOffset = _serializer.uint8(obj.block_id, buffer, bufferOffset);
    // Serialize message field [block_state]
    bufferOffset = _serializer.uint8(obj.block_state, buffer, bufferOffset);
    // Serialize message field [flags]
    bufferOffset = _arraySerializer.uint8(obj.flags, buffer, bufferOffset, null);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type MapC
    let len;
    let data = new MapC(null);
    // Deserialize message field [f_id]
    data.f_id = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [block_id]
    data.block_id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [block_state]
    data.block_state = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [flags]
    data.flags = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.flags.length;
    return length + 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/MapC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f846807ed51661abe48fa122b2e37650';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint16 f_id
    uint8 block_id #1-8
    uint8 block_state #0: unknown 1:occ 2:free 3:mix
    uint8[] flags #00 00 00 00  (free occ)(free occ)(free occ)(free occ)
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new MapC(null);
    if (msg.f_id !== undefined) {
      resolved.f_id = msg.f_id;
    }
    else {
      resolved.f_id = 0
    }

    if (msg.block_id !== undefined) {
      resolved.block_id = msg.block_id;
    }
    else {
      resolved.block_id = 0
    }

    if (msg.block_state !== undefined) {
      resolved.block_state = msg.block_state;
    }
    else {
      resolved.block_state = 0
    }

    if (msg.flags !== undefined) {
      resolved.flags = msg.flags;
    }
    else {
      resolved.flags = []
    }

    return resolved;
    }
};

module.exports = MapC;
