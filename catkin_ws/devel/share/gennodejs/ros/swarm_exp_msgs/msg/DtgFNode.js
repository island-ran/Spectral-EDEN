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

class DtgFNode {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.f_id = null;
      this.alive = null;
      this.vp_flags = null;
      this.need_help = null;
    }
    else {
      if (initObj.hasOwnProperty('f_id')) {
        this.f_id = initObj.f_id
      }
      else {
        this.f_id = 0;
      }
      if (initObj.hasOwnProperty('alive')) {
        this.alive = initObj.alive
      }
      else {
        this.alive = false;
      }
      if (initObj.hasOwnProperty('vp_flags')) {
        this.vp_flags = initObj.vp_flags
      }
      else {
        this.vp_flags = [];
      }
      if (initObj.hasOwnProperty('need_help')) {
        this.need_help = initObj.need_help
      }
      else {
        this.need_help = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgFNode
    // Serialize message field [f_id]
    bufferOffset = _serializer.uint16(obj.f_id, buffer, bufferOffset);
    // Serialize message field [alive]
    bufferOffset = _serializer.bool(obj.alive, buffer, bufferOffset);
    // Serialize message field [vp_flags]
    bufferOffset = _arraySerializer.uint8(obj.vp_flags, buffer, bufferOffset, null);
    // Serialize message field [need_help]
    bufferOffset = _serializer.uint8(obj.need_help, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgFNode
    let len;
    let data = new DtgFNode(null);
    // Deserialize message field [f_id]
    data.f_id = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [alive]
    data.alive = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [vp_flags]
    data.vp_flags = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [need_help]
    data.need_help = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.vp_flags.length;
    return length + 8;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/DtgFNode';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'f35023aa28359878229c76466f958b4c';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    const resolved = new DtgFNode(null);
    if (msg.f_id !== undefined) {
      resolved.f_id = msg.f_id;
    }
    else {
      resolved.f_id = 0
    }

    if (msg.alive !== undefined) {
      resolved.alive = msg.alive;
    }
    else {
      resolved.alive = false
    }

    if (msg.vp_flags !== undefined) {
      resolved.vp_flags = msg.vp_flags;
    }
    else {
      resolved.vp_flags = []
    }

    if (msg.need_help !== undefined) {
      resolved.need_help = msg.need_help;
    }
    else {
      resolved.need_help = 0
    }

    return resolved;
    }
};

module.exports = DtgFNode;
