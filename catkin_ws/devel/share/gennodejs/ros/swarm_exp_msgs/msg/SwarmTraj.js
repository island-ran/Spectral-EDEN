// Auto-generated. Do not edit!

// (in-package swarm_exp_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let geometry_msgs = _finder('geometry_msgs');

//-----------------------------------------------------------

class SwarmTraj {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.from_uav = null;
      this.start_t = null;
      this.order_p = null;
      this.t_p = null;
      this.coef_p = null;
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
      if (initObj.hasOwnProperty('start_t')) {
        this.start_t = initObj.start_t
      }
      else {
        this.start_t = 0.0;
      }
      if (initObj.hasOwnProperty('order_p')) {
        this.order_p = initObj.order_p
      }
      else {
        this.order_p = 0;
      }
      if (initObj.hasOwnProperty('t_p')) {
        this.t_p = initObj.t_p
      }
      else {
        this.t_p = [];
      }
      if (initObj.hasOwnProperty('coef_p')) {
        this.coef_p = initObj.coef_p
      }
      else {
        this.coef_p = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SwarmTraj
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [start_t]
    bufferOffset = _serializer.float64(obj.start_t, buffer, bufferOffset);
    // Serialize message field [order_p]
    bufferOffset = _serializer.int8(obj.order_p, buffer, bufferOffset);
    // Serialize message field [t_p]
    bufferOffset = _arraySerializer.float32(obj.t_p, buffer, bufferOffset, null);
    // Serialize message field [coef_p]
    // Serialize the length for message field [coef_p]
    bufferOffset = _serializer.uint32(obj.coef_p.length, buffer, bufferOffset);
    obj.coef_p.forEach((val) => {
      bufferOffset = geometry_msgs.msg.Point.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SwarmTraj
    let len;
    let data = new SwarmTraj(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [start_t]
    data.start_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [order_p]
    data.order_p = _deserializer.int8(buffer, bufferOffset);
    // Deserialize message field [t_p]
    data.t_p = _arrayDeserializer.float32(buffer, bufferOffset, null)
    // Deserialize message field [coef_p]
    // Deserialize array length for message field [coef_p]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.coef_p = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.coef_p[i] = geometry_msgs.msg.Point.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    length += 4 * object.t_p.length;
    length += 24 * object.coef_p.length;
    return length + 22;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/SwarmTraj';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ad6c481b3a1cd3eae78b78f76fc6821d';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # immediate for local or group
    
    uint8[] to_uavs
    uint8 from_uav
    
    float64 start_t
    int8 order_p
    float32[] t_p
    geometry_msgs/Point[] coef_p
    
    
    ================================================================================
    MSG: geometry_msgs/Point
    # This contains the position of a point in free space
    float64 x
    float64 y
    float64 z
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SwarmTraj(null);
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

    if (msg.start_t !== undefined) {
      resolved.start_t = msg.start_t;
    }
    else {
      resolved.start_t = 0.0
    }

    if (msg.order_p !== undefined) {
      resolved.order_p = msg.order_p;
    }
    else {
      resolved.order_p = 0
    }

    if (msg.t_p !== undefined) {
      resolved.t_p = msg.t_p;
    }
    else {
      resolved.t_p = []
    }

    if (msg.coef_p !== undefined) {
      resolved.coef_p = new Array(msg.coef_p.length);
      for (let i = 0; i < resolved.coef_p.length; ++i) {
        resolved.coef_p[i] = geometry_msgs.msg.Point.Resolve(msg.coef_p[i]);
      }
    }
    else {
      resolved.coef_p = []
    }

    return resolved;
    }
};

module.exports = SwarmTraj;
