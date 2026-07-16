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

class GroupState {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.from_uav = null;
      this.cur_t = null;
      this.group_size = null;
      this.trooper_targets = null;
      this.potential = null;
      this.coverage_work_load = null;
      this.state = null;
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
      if (initObj.hasOwnProperty('group_size')) {
        this.group_size = initObj.group_size
      }
      else {
        this.group_size = 0;
      }
      if (initObj.hasOwnProperty('trooper_targets')) {
        this.trooper_targets = initObj.trooper_targets
      }
      else {
        this.trooper_targets = [];
      }
      if (initObj.hasOwnProperty('potential')) {
        this.potential = initObj.potential
      }
      else {
        this.potential = 0;
      }
      if (initObj.hasOwnProperty('coverage_work_load')) {
        this.coverage_work_load = initObj.coverage_work_load
      }
      else {
        this.coverage_work_load = 0;
      }
      if (initObj.hasOwnProperty('state')) {
        this.state = initObj.state
      }
      else {
        this.state = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GroupState
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [cur_t]
    bufferOffset = _serializer.float64(obj.cur_t, buffer, bufferOffset);
    // Serialize message field [group_size]
    bufferOffset = _serializer.uint8(obj.group_size, buffer, bufferOffset);
    // Serialize message field [trooper_targets]
    bufferOffset = _arraySerializer.uint32(obj.trooper_targets, buffer, bufferOffset, null);
    // Serialize message field [potential]
    bufferOffset = _serializer.uint16(obj.potential, buffer, bufferOffset);
    // Serialize message field [coverage_work_load]
    bufferOffset = _serializer.uint16(obj.coverage_work_load, buffer, bufferOffset);
    // Serialize message field [state]
    bufferOffset = _serializer.uint8(obj.state, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GroupState
    let len;
    let data = new GroupState(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [cur_t]
    data.cur_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [group_size]
    data.group_size = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [trooper_targets]
    data.trooper_targets = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [potential]
    data.potential = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [coverage_work_load]
    data.coverage_work_load = _deserializer.uint16(buffer, bufferOffset);
    // Deserialize message field [state]
    data.state = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    length += 4 * object.trooper_targets.length;
    return length + 23;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/GroupState';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '17f63ea80422701593a5c33c707b7615';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # inter group broad cast (slow timer)
    uint8[] to_uavs
    uint8 from_uav
    
    float64 cur_t
    uint8 group_size
    uint32[] trooper_targets
    uint16 potential #calculated unknown space number, for helpers
    uint16 coverage_work_load
    uint8 state # 0000 00(need help)(exp/cover)
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GroupState(null);
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

    if (msg.group_size !== undefined) {
      resolved.group_size = msg.group_size;
    }
    else {
      resolved.group_size = 0
    }

    if (msg.trooper_targets !== undefined) {
      resolved.trooper_targets = msg.trooper_targets;
    }
    else {
      resolved.trooper_targets = []
    }

    if (msg.potential !== undefined) {
      resolved.potential = msg.potential;
    }
    else {
      resolved.potential = 0
    }

    if (msg.coverage_work_load !== undefined) {
      resolved.coverage_work_load = msg.coverage_work_load;
    }
    else {
      resolved.coverage_work_load = 0
    }

    if (msg.state !== undefined) {
      resolved.state = msg.state;
    }
    else {
      resolved.state = 0
    }

    return resolved;
    }
};

module.exports = GroupState;
