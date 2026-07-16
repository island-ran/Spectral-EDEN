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

class GroupCommandReq {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.cur_t = null;
      this.dispatch_type = null;
      this.departure_EROIs = null;
      this.departure_status = null;
      this.helped_group_id = null;
    }
    else {
      if (initObj.hasOwnProperty('to_uavs')) {
        this.to_uavs = initObj.to_uavs
      }
      else {
        this.to_uavs = [];
      }
      if (initObj.hasOwnProperty('cur_t')) {
        this.cur_t = initObj.cur_t
      }
      else {
        this.cur_t = 0.0;
      }
      if (initObj.hasOwnProperty('dispatch_type')) {
        this.dispatch_type = initObj.dispatch_type
      }
      else {
        this.dispatch_type = 0;
      }
      if (initObj.hasOwnProperty('departure_EROIs')) {
        this.departure_EROIs = initObj.departure_EROIs
      }
      else {
        this.departure_EROIs = [];
      }
      if (initObj.hasOwnProperty('departure_status')) {
        this.departure_status = initObj.departure_status
      }
      else {
        this.departure_status = 0;
      }
      if (initObj.hasOwnProperty('helped_group_id')) {
        this.helped_group_id = initObj.helped_group_id
      }
      else {
        this.helped_group_id = 0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type GroupCommandReq
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [cur_t]
    bufferOffset = _serializer.float64(obj.cur_t, buffer, bufferOffset);
    // Serialize message field [dispatch_type]
    bufferOffset = _serializer.uint8(obj.dispatch_type, buffer, bufferOffset);
    // Serialize message field [departure_EROIs]
    bufferOffset = _arraySerializer.uint32(obj.departure_EROIs, buffer, bufferOffset, null);
    // Serialize message field [departure_status]
    bufferOffset = _serializer.uint8(obj.departure_status, buffer, bufferOffset);
    // Serialize message field [helped_group_id]
    bufferOffset = _serializer.uint8(obj.helped_group_id, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type GroupCommandReq
    let len;
    let data = new GroupCommandReq(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [cur_t]
    data.cur_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [dispatch_type]
    data.dispatch_type = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [departure_EROIs]
    data.departure_EROIs = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [departure_status]
    data.departure_status = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [helped_group_id]
    data.helped_group_id = _deserializer.uint8(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    length += 4 * object.departure_EROIs.length;
    return length + 19;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/GroupCommandReq';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '8907e16fca4b37e09a5750aaba75e3cc';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # inside group, from trooper (Quick timmer)
    uint8[] to_uavs
    # uint8 from_uav
    
    float64 cur_t
    # 1: Departure; 2: Help, 3: switch trooper
    uint8 dispatch_type 
    
    # departure
    uint32[] departure_EROIs
    uint8 departure_status #0: trooper; 1: infantry
    
    #help
    uint8 helped_group_id
    
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new GroupCommandReq(null);
    if (msg.to_uavs !== undefined) {
      resolved.to_uavs = msg.to_uavs;
    }
    else {
      resolved.to_uavs = []
    }

    if (msg.cur_t !== undefined) {
      resolved.cur_t = msg.cur_t;
    }
    else {
      resolved.cur_t = 0.0
    }

    if (msg.dispatch_type !== undefined) {
      resolved.dispatch_type = msg.dispatch_type;
    }
    else {
      resolved.dispatch_type = 0
    }

    if (msg.departure_EROIs !== undefined) {
      resolved.departure_EROIs = msg.departure_EROIs;
    }
    else {
      resolved.departure_EROIs = []
    }

    if (msg.departure_status !== undefined) {
      resolved.departure_status = msg.departure_status;
    }
    else {
      resolved.departure_status = 0
    }

    if (msg.helped_group_id !== undefined) {
      resolved.helped_group_id = msg.helped_group_id;
    }
    else {
      resolved.helped_group_id = 0
    }

    return resolved;
    }
};

module.exports = GroupCommandReq;
