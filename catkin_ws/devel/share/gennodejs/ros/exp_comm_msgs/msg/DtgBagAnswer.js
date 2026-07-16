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

class DtgBagAnswer {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.id = null;
      this.to_uav = null;
      this.bag_id = null;
    }
    else {
      if (initObj.hasOwnProperty('id')) {
        this.id = initObj.id
      }
      else {
        this.id = 0;
      }
      if (initObj.hasOwnProperty('to_uav')) {
        this.to_uav = initObj.to_uav
      }
      else {
        this.to_uav = 0;
      }
      if (initObj.hasOwnProperty('bag_id')) {
        this.bag_id = initObj.bag_id
      }
      else {
        this.bag_id = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type DtgBagAnswer
    // Serialize message field [id]
    bufferOffset = _serializer.uint8(obj.id, buffer, bufferOffset);
    // Serialize message field [to_uav]
    bufferOffset = _serializer.uint8(obj.to_uav, buffer, bufferOffset);
    // Serialize message field [bag_id]
    bufferOffset = _serializer.uint32(obj.bag_id, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type DtgBagAnswer
    let len;
    let data = new DtgBagAnswer(null);
    // Deserialize message field [id]
    data.id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [to_uav]
    data.to_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [bag_id]
    data.bag_id = _deserializer.uint32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 6;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/DtgBagAnswer';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '38ef4e8bfd92de589cce314b18d25dfb';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    uint8 id
    uint8 to_uav
    uint32 bag_id
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new DtgBagAnswer(null);
    if (msg.id !== undefined) {
      resolved.id = msg.id;
    }
    else {
      resolved.id = 0
    }

    if (msg.to_uav !== undefined) {
      resolved.to_uav = msg.to_uav;
    }
    else {
      resolved.to_uav = 0
    }

    if (msg.bag_id !== undefined) {
      resolved.bag_id = msg.bag_id;
    }
    else {
      resolved.bag_id = 0
    }

    return resolved;
    }
};

module.exports = DtgBagAnswer;
