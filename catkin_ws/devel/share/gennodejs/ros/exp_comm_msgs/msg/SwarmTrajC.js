// Auto-generated. Do not edit!

// (in-package exp_comm_msgs.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let geometry_msgs = _finder('geometry_msgs');

//-----------------------------------------------------------

class SwarmTrajC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.pub_t = null;
      this.id = null;
      this.start_t = null;
      this.order_p = null;
      this.t_p = null;
      this.coef_p = null;
    }
    else {
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('id')) {
        this.id = initObj.id
      }
      else {
        this.id = 0;
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
    // Serializes a message object of type SwarmTrajC
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [id]
    bufferOffset = _serializer.uint8(obj.id, buffer, bufferOffset);
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
    //deserializes a message object of type SwarmTrajC
    let len;
    let data = new SwarmTrajC(null);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [id]
    data.id = _deserializer.uint8(buffer, bufferOffset);
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
    length += 4 * object.t_p.length;
    length += 24 * object.coef_p.length;
    return length + 26;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/SwarmTrajC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'b412e83c14c239c5f9f2ca748f0bead0';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 pub_t
    
    uint8 id
    
    float64 start_t
    int8 order_p
    float32[] t_p
    geometry_msgs/Point[] coef_p
    
    # int8 order_yaw
    # float32[] t_yaw
    # float32[] coef_yaw
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
    const resolved = new SwarmTrajC(null);
    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.id !== undefined) {
      resolved.id = msg.id;
    }
    else {
      resolved.id = 0
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

module.exports = SwarmTrajC;
