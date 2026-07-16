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

class SwarmJobC {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.pub_t = null;
      this.from_uav = null;
      this.JobState = null;
      this.target_fn = null;
      this.target_hn = null;
      this.dist_to_fn = null;
      this.dist_to_hn = null;
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
      if (initObj.hasOwnProperty('JobState')) {
        this.JobState = initObj.JobState
      }
      else {
        this.JobState = 0;
      }
      if (initObj.hasOwnProperty('target_fn')) {
        this.target_fn = initObj.target_fn
      }
      else {
        this.target_fn = 0;
      }
      if (initObj.hasOwnProperty('target_hn')) {
        this.target_hn = initObj.target_hn
      }
      else {
        this.target_hn = 0;
      }
      if (initObj.hasOwnProperty('dist_to_fn')) {
        this.dist_to_fn = initObj.dist_to_fn
      }
      else {
        this.dist_to_fn = 0.0;
      }
      if (initObj.hasOwnProperty('dist_to_hn')) {
        this.dist_to_hn = initObj.dist_to_hn
      }
      else {
        this.dist_to_hn = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type SwarmJobC
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [from_uav]
    bufferOffset = _serializer.uint8(obj.from_uav, buffer, bufferOffset);
    // Serialize message field [JobState]
    bufferOffset = _serializer.uint8(obj.JobState, buffer, bufferOffset);
    // Serialize message field [target_fn]
    bufferOffset = _serializer.int32(obj.target_fn, buffer, bufferOffset);
    // Serialize message field [target_hn]
    bufferOffset = _serializer.int32(obj.target_hn, buffer, bufferOffset);
    // Serialize message field [dist_to_fn]
    bufferOffset = _serializer.float32(obj.dist_to_fn, buffer, bufferOffset);
    // Serialize message field [dist_to_hn]
    bufferOffset = _serializer.float32(obj.dist_to_hn, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type SwarmJobC
    let len;
    let data = new SwarmJobC(null);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [from_uav]
    data.from_uav = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [JobState]
    data.JobState = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [target_fn]
    data.target_fn = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [target_hn]
    data.target_hn = _deserializer.int32(buffer, bufferOffset);
    // Deserialize message field [dist_to_fn]
    data.dist_to_fn = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [dist_to_hn]
    data.dist_to_hn = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 26;
  }

  static datatype() {
    // Returns string type for a message object
    return 'exp_comm_msgs/SwarmJobC';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'e25c09eda529caf2748eb82a8886575b';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    float64 pub_t
    uint8 from_uav
    uint8 JobState #0000 0(no job = 0/ new job = 1)(finish = 1)(local = 0/ global = 1)
    int32  target_fn
    int32 target_hn
    float32 dist_to_fn
    float32 dist_to_hn
    
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new SwarmJobC(null);
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

    if (msg.JobState !== undefined) {
      resolved.JobState = msg.JobState;
    }
    else {
      resolved.JobState = 0
    }

    if (msg.target_fn !== undefined) {
      resolved.target_fn = msg.target_fn;
    }
    else {
      resolved.target_fn = 0
    }

    if (msg.target_hn !== undefined) {
      resolved.target_hn = msg.target_hn;
    }
    else {
      resolved.target_hn = 0
    }

    if (msg.dist_to_fn !== undefined) {
      resolved.dist_to_fn = msg.dist_to_fn;
    }
    else {
      resolved.dist_to_fn = 0.0
    }

    if (msg.dist_to_hn !== undefined) {
      resolved.dist_to_hn = msg.dist_to_hn;
    }
    else {
      resolved.dist_to_hn = 0.0
    }

    return resolved;
    }
};

module.exports = SwarmJobC;
