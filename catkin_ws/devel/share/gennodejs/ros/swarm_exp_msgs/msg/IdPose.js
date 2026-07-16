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

class IdPose {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.to_uavs = null;
      this.id = null;
      this.pub_t = null;
      this.pose = null;
    }
    else {
      if (initObj.hasOwnProperty('to_uavs')) {
        this.to_uavs = initObj.to_uavs
      }
      else {
        this.to_uavs = [];
      }
      if (initObj.hasOwnProperty('id')) {
        this.id = initObj.id
      }
      else {
        this.id = 0;
      }
      if (initObj.hasOwnProperty('pub_t')) {
        this.pub_t = initObj.pub_t
      }
      else {
        this.pub_t = 0.0;
      }
      if (initObj.hasOwnProperty('pose')) {
        this.pose = initObj.pose
      }
      else {
        this.pose = new geometry_msgs.msg.Pose();
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type IdPose
    // Serialize message field [to_uavs]
    bufferOffset = _arraySerializer.uint8(obj.to_uavs, buffer, bufferOffset, null);
    // Serialize message field [id]
    bufferOffset = _serializer.uint8(obj.id, buffer, bufferOffset);
    // Serialize message field [pub_t]
    bufferOffset = _serializer.float64(obj.pub_t, buffer, bufferOffset);
    // Serialize message field [pose]
    bufferOffset = geometry_msgs.msg.Pose.serialize(obj.pose, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type IdPose
    let len;
    let data = new IdPose(null);
    // Deserialize message field [to_uavs]
    data.to_uavs = _arrayDeserializer.uint8(buffer, bufferOffset, null)
    // Deserialize message field [id]
    data.id = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [pub_t]
    data.pub_t = _deserializer.float64(buffer, bufferOffset);
    // Deserialize message field [pose]
    data.pose = geometry_msgs.msg.Pose.deserialize(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += object.to_uavs.length;
    return length + 69;
  }

  static datatype() {
    // Returns string type for a message object
    return 'swarm_exp_msgs/IdPose';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'cc5f67458fe878e0f625ae446543e38e';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # immediate for local or group, slow timer for general
    uint8[] to_uavs
    
    uint8 id
    float64 pub_t
    geometry_msgs/Pose pose
    ================================================================================
    MSG: geometry_msgs/Pose
    # A representation of pose in free space, composed of position and orientation. 
    Point position
    Quaternion orientation
    
    ================================================================================
    MSG: geometry_msgs/Point
    # This contains the position of a point in free space
    float64 x
    float64 y
    float64 z
    
    ================================================================================
    MSG: geometry_msgs/Quaternion
    # This represents an orientation in free space in quaternion form.
    
    float64 x
    float64 y
    float64 z
    float64 w
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new IdPose(null);
    if (msg.to_uavs !== undefined) {
      resolved.to_uavs = msg.to_uavs;
    }
    else {
      resolved.to_uavs = []
    }

    if (msg.id !== undefined) {
      resolved.id = msg.id;
    }
    else {
      resolved.id = 0
    }

    if (msg.pub_t !== undefined) {
      resolved.pub_t = msg.pub_t;
    }
    else {
      resolved.pub_t = 0.0
    }

    if (msg.pose !== undefined) {
      resolved.pose = geometry_msgs.msg.Pose.Resolve(msg.pose)
    }
    else {
      resolved.pose = new geometry_msgs.msg.Pose()
    }

    return resolved;
    }
};

module.exports = IdPose;
