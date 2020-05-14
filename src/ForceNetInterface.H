/**
 * \author Daniel Keefe (dfk)
 *
 * \file  ForceNetInterface.H
 * \brief An interface for talking to a Force Device server over the network.
 *
 */

 /*
#ifndef FORCENETINTERFACE_H
#define FORCENETINTERFACE_H

//#include "VRBase.H"
*
#define FORCENETINTERFACE_DEFAULT_PORT 1205

namespace DrawOnAir {



class ForceNetInterface : public InputDevice
{
public:

  enum ForceNetMessageType {
    NO_MSG = 0,
    StopForces = 1001,
    TapeDrawing = 1002,
    ReverseTapeDrawing = 1003,
    OneHandDrawing = 1004,
    HandOffset = 1005,
    PlaneEffect = 1006,
    StopPlaneEffect = 1007,
    MovingPlaneDrawing = 1008,
    ViscousEffect = 1009,
    FrictionEffect = 1010,
    SetGeometry = 1011,
    ClearGeometry = 1012,
    HybridDrawing = 1013,
    HybridDrawingTransition = 1014,
    PlaySound = 1015,
    AnisotropicFilter = 1016,
    DynamicDraggingCurv = 1017,
    Shutdown = 1018
  };

  ForceNetInterface(const std::string &host, int port=FORCENETINTERFACE_DEFAULT_PORT);
  virtual ~ForceNetInterface();

  void stopForces();
  void startTapeDrawing(bool allowRedraw);
  void startReverseTapeDrawing();
  void setHandOffset(G3D::Vector3 newOffset);
  void startPlaneEffect();
  void stopPlaneEffect();
  void startOneHandDrawing();
  void startMovingPlaneDrawing();
  void setViscousGain(float gain);
  void turnFrictionOn();
  void turnFrictionOff();
  void startHybridDrawing();
  void hybridDrawingTransition();
  void playSound(const std::string &soundFilename);
  void startAnisotropicFilter();
  void toggleDynamicDragging();
  void shutdown();


  void setGeometry(int id, G3D::RenderDevice::Primitive type, const G3D::Array<G3D::Vector3> &verts);
  void clearGeometry(int id);

  void pollForInput(G3D::Array<VRG3D::EventRef> &events);

protected:
  G3D::NetworkDevice         *_networkDevice;
  G3D::ReliableConduitRef     _conduit;
  G3D::NetAddress             _address;

private:
  ForceNetInterface(const ForceNetInterface &);
  ForceNetInterface& operator=(const ForceNetInterface &);
};


class ForceNetMsg_StopForces 
{
public:
  ForceNetMsg_StopForces() {}
  virtual ~ForceNetMsg_StopForces() {}

  G3D::uint32 type() const { return ForceNetInterface::StopForces; }  
  
  void serialize(G3D::BinaryOutput &b) const { }  
  void deserialize(G3D::BinaryInput &b) { }
};

class ForceNetMsg_TapeDrawing 
{
public:
  ForceNetMsg_TapeDrawing(bool allowRedraw) { redraw = allowRedraw; }
  virtual ~ForceNetMsg_TapeDrawing() {}

  G3D::uint32 type() const { return ForceNetInterface::TapeDrawing; }  
  
  void serialize(G3D::BinaryOutput &b) const { b.writeBool8(redraw); }
  void deserialize(G3D::BinaryInput &b) { redraw = b.readBool8(); }

  bool redraw;
};

class ForceNetMsg_DynamicDraggingCurv 
{
public:
  ForceNetMsg_DynamicDraggingCurv() {}
  virtual ~ForceNetMsg_DynamicDraggingCurv() {}

  G3D::uint32 type() const { return ForceNetInterface::DynamicDraggingCurv; }  
  
  void serialize(G3D::BinaryOutput &b) const { }
  void deserialize(G3D::BinaryInput &b) { }
};

class ForceNetMsg_ReverseTapeDrawing 
{
public:
  ForceNetMsg_ReverseTapeDrawing() {}
  virtual ~ForceNetMsg_ReverseTapeDrawing() {}

  G3D::uint32 type() const { return ForceNetInterface::ReverseTapeDrawing; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_AnisotropicFilter
{
public:
  ForceNetMsg_AnisotropicFilter() {}
  virtual ~ForceNetMsg_AnisotropicFilter() {}

  G3D::uint32 type() const { return ForceNetInterface::AnisotropicFilter; }
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_HandOffset 
{
public:
  ForceNetMsg_HandOffset(G3D::Vector3 newOffset) { handOffset = newOffset; }
  virtual ~ForceNetMsg_HandOffset() {}

  G3D::uint32 type() const { return ForceNetInterface::HandOffset; }  
  
  void serialize(G3D::BinaryOutput &b) const { handOffset.serialize(b); }
  void deserialize(G3D::BinaryInput &b) { handOffset.deserialize(b); }

  G3D::Vector3 handOffset;
};

class ForceNetMsg_PlaneEffect 
{
public:
  ForceNetMsg_PlaneEffect() {}
  virtual ~ForceNetMsg_PlaneEffect() {}

  G3D::uint32 type() const { return ForceNetInterface::PlaneEffect; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_StopPlaneEffect 
{
public:
  ForceNetMsg_StopPlaneEffect() {}
  virtual ~ForceNetMsg_StopPlaneEffect() {}

  G3D::uint32 type() const { return ForceNetInterface::StopPlaneEffect; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_MovingPlaneDrawing 
{
public:
  ForceNetMsg_MovingPlaneDrawing() {}
  virtual ~ForceNetMsg_MovingPlaneDrawing() {}

  G3D::uint32 type() const { return ForceNetInterface::MovingPlaneDrawing; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_OneHandDrawing 
{
public:
  ForceNetMsg_OneHandDrawing() {}
  virtual ~ForceNetMsg_OneHandDrawing() {}

  G3D::uint32 type() const { return ForceNetInterface::OneHandDrawing; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_FrictionEffect 
{
public:
  ForceNetMsg_FrictionEffect(int on) { onoff = on; }
  virtual ~ForceNetMsg_FrictionEffect() {}

  G3D::uint32 type() const { return ForceNetInterface::FrictionEffect; }  
  
  void serialize(G3D::BinaryOutput &b) const { b.writeInt32(onoff); }
  void deserialize(G3D::BinaryInput &b) { onoff = b.readInt32(); }

  int onoff;
};

class ForceNetMsg_ViscousEffect 
{
public:
  ForceNetMsg_ViscousEffect(float newGain) { gain = newGain; }
  virtual ~ForceNetMsg_ViscousEffect() {}

  G3D::uint32 type() const { return ForceNetInterface::ViscousEffect; }  
  
  void serialize(G3D::BinaryOutput &b) const { b.writeFloat32(gain); }
  void deserialize(G3D::BinaryInput &b) { gain = b.readFloat32(); }
  
  float gain;
};

class ForceNetMsg_SetGeometry 
{
public:
  ForceNetMsg_SetGeometry(int geomID, G3D::RenderDevice::Primitive geomPrim, const G3D::Array<G3D::Vector3> &geomVerts) {
    id = geomID;
    prim = geomPrim;
    verts = geomVerts;
  }
  ForceNetMsg_SetGeometry() {}
  virtual ~ForceNetMsg_SetGeometry() {}

  G3D::uint32 type() const { return ForceNetInterface::SetGeometry; }  
  
  void serialize(G3D::BinaryOutput &b) const {
    b.writeInt32(id);
    b.writeInt32((int)prim);
    b.writeInt32(verts.size());
    for (int i=0;i<verts.size();i++) {
      verts[i].serialize(b);
    }
  }

  void deserialize(G3D::BinaryInput &b) {
    id = b.readInt32(); 
    prim = (G3D::RenderDevice::Primitive)b.readInt32();
    int n = b.readInt32(); 
    for (int i=0;i<n;i++) {
      G3D::Vector3 v;
      v.deserialize(b);
      verts.append(v);
    }  
  }

  int id;
  G3D::RenderDevice::Primitive prim;
  G3D::Array<G3D::Vector3> verts;
};

class ForceNetMsg_ClearGeometry 
{
public:
  ForceNetMsg_ClearGeometry(int geomID) {
    id = geomID;
  }
  virtual ~ForceNetMsg_ClearGeometry() {}

  G3D::uint32 type() const { return ForceNetInterface::ClearGeometry; }  
  
  void serialize(G3D::BinaryOutput &b) const {
    b.writeInt32(id);
  }

  void deserialize(G3D::BinaryInput &b) {
    id = b.readInt32(); 
  }

  int id;
};

class ForceNetMsg_HybridDrawing 
{
public:
  ForceNetMsg_HybridDrawing() {}
  virtual ~ForceNetMsg_HybridDrawing() {}

  G3D::uint32 type() const { return ForceNetInterface::HybridDrawing; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_HybridDrawingTransition 
{
public:
  ForceNetMsg_HybridDrawingTransition() {}
  virtual ~ForceNetMsg_HybridDrawingTransition() {}

  G3D::uint32 type() const { return ForceNetInterface::HybridDrawingTransition; }  
  
  void serialize(G3D::BinaryOutput &b) const {}
  void deserialize(G3D::BinaryInput &b) {}
};

class ForceNetMsg_PlaySound 
{
public:
  ForceNetMsg_PlaySound(const std::string &soundFilename) {
    filename = soundFilename;
  }
  virtual ~ForceNetMsg_PlaySound() {}

  G3D::uint32 type() const { return ForceNetInterface::PlaySound; }  
  
  void serialize(G3D::BinaryOutput &b) const {
    b.writeString(filename);
  }

  void deserialize(G3D::BinaryInput &b) {
    filename = b.readString();
  }

  std::string filename;
};

class ForceNetMsg_Shutdown
{
public:
  ForceNetMsg_Shutdown() {}
  virtual ~ForceNetMsg_Shutdown() {}

  G3D::uint32 type() const { return ForceNetInterface::Shutdown; }  
  
  void serialize(G3D::BinaryOutput &b) const {}

  void deserialize(G3D::BinaryInput &b) {}
};

} // end namespace

#endif
*/
