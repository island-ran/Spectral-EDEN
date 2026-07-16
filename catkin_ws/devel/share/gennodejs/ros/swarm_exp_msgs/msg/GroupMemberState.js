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

class GroupMemberState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.from_uav = null;
      this.cur_t = null;
      this.targets = null;
      this.work_type = null;
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
      if (initObj.hasOwnProperty('cur_t')) {
        this.cur_t = initObj.cur_t
      }
      else {
        this.cur_t = 0.0;
      }
      if (initObj.hasOwnProperty('targets')) {
        this.targets = initObj.targets
      }
      else {
        this.targets = [];
      }
      if (initObj.hasOwnProperty('work_type')) {
        this.work_type = initObj.work_type
      }
      else {
        this.work_type = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GroupMemberState
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [cur_t]
    bufferOffset = _serializer.float64(obj.cur_t, buffer, bufferOffset);
    // Serialize message field [targets]
    bufferOffset = _arraySerializer.uint32(obj.targets, buffer, bufferOffset, null);
    // Serialize message field [work_type]
    bufferOffset = _serializer.uint8(obj.work_type, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GroupMemberState
    let len;
    let data = new GroupMemberState(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [cur_t]
    data.cur_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [targets]
    data.targets = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [work_type]
    data.work_type = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    length += 4 * object.targets.length;
    return length + 18;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/GroupMemberState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'abb6338d4d1acda965b2e312dd0c2bb4';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # group broad cast (quick timer)
    uint8[] to_uavs
    uint8 from_uav
    
    float64 cur_t
    uint32[] targets
    uint8 work_type #cover, explore, follow, wait
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GroupMemberState(null);
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

    if (msg.cur_t !== undefined) {
      resolved.cur_t = msg.cur_t;
    }
    else {
      resolved.cur_t = 0.0
    }

    if (msg.targets !== undefined) {
      resolved.targets = msg.targets;
    }
    else {
      resolved.targets = []
    }

    if (msg.work_type !== undefined) {
      resolved.work_type = msg.work_type;
    }
    else {
      resolved.work_type = 0
    }

    return resolved;
    }
};

module.exports = GroupMemberState;
