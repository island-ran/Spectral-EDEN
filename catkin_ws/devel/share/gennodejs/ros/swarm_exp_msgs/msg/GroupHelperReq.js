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

class GroupHelperReq {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.from_uav = null;
      this.helper_ids = null;
      this.cost = null;
    }
    else {
      if (initObj.hasOwnProperty('to_uavs')) {
        this.to_uavs = initObj.to_uavs
      }
      else {
        this.to_uavs = [];
      }
      if (initObj.hasOwnProperty('from_uav')) {
        this.from_uav = initObj.from_uav
      }
      else {
        this.from_uav = 0;
      }
      if (initObj.hasOwnProperty('helper_ids')) {
        this.helper_ids = initObj.helper_ids
      }
      else {
        this.helper_ids = [];
      }
      if (initObj.hasOwnProperty('cost')) {
        this.cost = initObj.cost
      }
      else {
        this.cost = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GroupHelperReq
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [helper_ids]
    bufferOffset = _arraySerializer.uint8(obj.helper_ids, buffer, bufferOffset, null);
    // Serialize message field [cost]
    bufferOffset = _serializer.float32(obj.cost, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GroupHelperReq
    let len;
    let data = new GroupHelperReq(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [helper_ids]
    data.helper_ids = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [cost]
    data.cost = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    length += object.helper_ids.length;
    return length + 13;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/GroupHelperReq';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'de1dd505e44758dd2e830a3b48f4bffe';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # to certain group trooper (immediate answer)
    uint8[] to_uavs
    uint8 from_uav
    
    uint8[] helper_ids
    float32 cost
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GroupHelperReq(null);
    if (msg.to_uavs !== undefined) {
      resolved.to_uavs = msg.to_uavs;
    }
    else {
      resolved.to_uavs = []
    }

    if (msg.from_uav !== undefined) {
      resolved.from_uav = msg.from_uav;
    }
    else {
      resolved.from_uav = 0
    }

    if (msg.helper_ids !== undefined) {
      resolved.helper_ids = msg.helper_ids;
    }
    else {
      resolved.helper_ids = []
    }

    if (msg.cost !== undefined) {
      resolved.cost = msg.cost;
    }
    else {
      resolved.cost = 0.0
    }

    return resolved;
    }
};

module.exports = GroupHelperReq;
