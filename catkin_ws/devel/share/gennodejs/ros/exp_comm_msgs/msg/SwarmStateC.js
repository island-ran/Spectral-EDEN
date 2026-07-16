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

class SwarmStateC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.pub_t = null;
      this.from_uav = null;
      this.flags = null;
      this.local_fn = null;
      this.dist_to_local_fn = null;
      this.connect_fn = null;
      this.dist_to_connect_fn = null;
      this.connect_hn = null;
      this.hn_pos_idx = null;
      this.dist_to_connect_hn = null;
    }
    else {
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('from_uav')) {
        this.from_uav = initObj.from_uav
      }
      else {
        this.from_uav = 0;
      }
      if (initObj.hasOwnProperty('flags')) {
        this.flags = initObj.flags
      }
      else {
        this.flags = 0;
      }
      if (initObj.hasOwnProperty('local_fn')) {
        this.local_fn = initObj.local_fn
      }
      else {
        this.local_fn = [];
      }
      if (initObj.hasOwnProperty('dist_to_local_fn')) {
        this.dist_to_local_fn = initObj.dist_to_local_fn
      }
      else {
        this.dist_to_local_fn = [];
      }
      if (initObj.hasOwnProperty('connect_fn')) {
        this.connect_fn = initObj.connect_fn
      }
      else {
        this.connect_fn = [];
      }
      if (initObj.hasOwnProperty('dist_to_connect_fn')) {
        this.dist_to_connect_fn = initObj.dist_to_connect_fn
      }
      else {
        this.dist_to_connect_fn = [];
      }
      if (initObj.hasOwnProperty('connect_hn')) {
        this.connect_hn = initObj.connect_hn
      }
      else {
        this.connect_hn = [];
      }
      if (initObj.hasOwnProperty('hn_pos_idx')) {
        this.hn_pos_idx = initObj.hn_pos_idx
      }
      else {
        this.hn_pos_idx = [];
      }
      if (initObj.hasOwnProperty('dist_to_connect_hn')) {
        this.dist_to_connect_hn = initObj.dist_to_connect_hn
      }
      else {
        this.dist_to_connect_hn = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SwarmStateC
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [flags]
    bufferOffset = _serializer.uint8(obj.flags, buffer, bufferOffset);
    // Serialize message field [local_fn]
    bufferOffset = _arraySerializer.uint16(obj.local_fn, buffer, bufferOffset, null);
    // Serialize message field [dist_to_local_fn]
    bufferOffset = _arraySerializer.float32(obj.dist_to_local_fn, buffer, bufferOffset, null);
    // Serialize message field [connect_fn]
    bufferOffset = _arraySerializer.uint16(obj.connect_fn, buffer, bufferOffset, null);
    // Serialize message field [dist_to_connect_fn]
    bufferOffset = _arraySerializer.float32(obj.dist_to_connect_fn, buffer, bufferOffset, null);
    // Serialize message field [connect_hn]
    bufferOffset = _arraySerializer.uint32(obj.connect_hn, buffer, bufferOffset, null);
    // Serialize message field [hn_pos_idx]
    bufferOffset = _arraySerializer.uint32(obj.hn_pos_idx, buffer, bufferOffset, null);
    // Serialize message field [dist_to_connect_hn]
    bufferOffset = _arraySerializer.float32(obj.dist_to_connect_hn, buffer, bufferOffset, null);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SwarmStateC
    let len;
    let data = new SwarmStateC(null);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [flags]
    data.flags = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [local_fn]
    data.local_fn = _arrayDeserializer.uint16(buffer, bufferOffset, null)
    // Deserialize message field [dist_to_local_fn]
    data.dist_to_local_fn = _arrayDeserializer.float32(buffer, bufferOffset, null)
    // Deserialize message field [connect_fn]
    data.connect_fn = _arrayDeserializer.uint16(buffer, bufferOffset, null)
    // Deserialize message field [dist_to_connect_fn]
    data.dist_to_connect_fn = _arrayDeserializer.float32(buffer, bufferOffset, null)
    // Deserialize message field [connect_hn]
    data.connect_hn = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [hn_pos_idx]
    data.hn_pos_idx = _arrayDeserializer.uint32(buffer, bufferOffset, null)
    // Deserialize message field [dist_to_connect_hn]
    data.dist_to_connect_hn = _arrayDeserializer.float32(buffer, bufferOffset, null)
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += 2 * object.local_fn.length;
    length += 4 * object.dist_to_local_fn.length;
    length += 2 * object.connect_fn.length;
    length += 4 * object.dist_to_connect_fn.length;
    length += 4 * object.connect_hn.length;
    length += 4 * object.hn_pos_idx.length;
    length += 4 * object.dist_to_connect_hn.length;
    return length + 38;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/SwarmStateC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return '11fc39f21c102f3f37a28c1e4132b63e';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 pub_t
    uint8 from_uav
    uint8 flags #0000 000(quick communication) 
    uint16[]  local_fn
    float32[] dist_to_local_fn
    
    uint16[]  connect_fn
    float32[] dist_to_connect_fn
    
    uint32[]  connect_hn
    uint32[]  hn_pos_idx
    float32[] dist_to_connect_hn  
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SwarmStateC(null);
    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.from_uav !== undefined) {
      resolved.from_uav = msg.from_uav;
    }
    else {
      resolved.from_uav = 0
    }

    if (msg.flags !== undefined) {
      resolved.flags = msg.flags;
    }
    else {
      resolved.flags = 0
    }

    if (msg.local_fn !== undefined) {
      resolved.local_fn = msg.local_fn;
    }
    else {
      resolved.local_fn = []
    }

    if (msg.dist_to_local_fn !== undefined) {
      resolved.dist_to_local_fn = msg.dist_to_local_fn;
    }
    else {
      resolved.dist_to_local_fn = []
    }

    if (msg.connect_fn !== undefined) {
      resolved.connect_fn = msg.connect_fn;
    }
    else {
      resolved.connect_fn = []
    }

    if (msg.dist_to_connect_fn !== undefined) {
      resolved.dist_to_connect_fn = msg.dist_to_connect_fn;
    }
    else {
      resolved.dist_to_connect_fn = []
    }

    if (msg.connect_hn !== undefined) {
      resolved.connect_hn = msg.connect_hn;
    }
    else {
      resolved.connect_hn = []
    }

    if (msg.hn_pos_idx !== undefined) {
      resolved.hn_pos_idx = msg.hn_pos_idx;
    }
    else {
      resolved.hn_pos_idx = []
    }

    if (msg.dist_to_connect_hn !== undefined) {
      resolved.dist_to_connect_hn = msg.dist_to_connect_hn;
    }
    else {
      resolved.dist_to_connect_hn = []
    }

    return resolved;
    }
};

module.exports = SwarmStateC;
