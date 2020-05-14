/*
#include "ForceNetInterface.H"
using namespace G3D;
namespace DrawOnAir {

ForceNetInterface::ForceNetInterface(const std::string &host, int port) : InputDevice()
{
  _networkDevice = NetworkDevice::instance();
  _address = NetAddress(host, port);
  _conduit = ReliableConduit::create(_address);
  alwaysAssertM(_conduit->ok(), "Conduit not ok.");
}

ForceNetInterface::~ForceNetInterface()
{
  _conduit = NULL;
  _networkDevice->cleanup();
  delete _networkDevice;
}

void
ForceNetInterface::stopForces()
{
  ForceNetMsg_StopForces m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startTapeDrawing(bool allowRedraw)
{
  ForceNetMsg_TapeDrawing m(allowRedraw);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startAnisotropicFilter()
{
  ForceNetMsg_AnisotropicFilter m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startReverseTapeDrawing()
{
  ForceNetMsg_ReverseTapeDrawing m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::setHandOffset(Vector3 newOffset)
{
  ForceNetMsg_HandOffset m(newOffset);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startPlaneEffect()
{
  ForceNetMsg_PlaneEffect m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::stopPlaneEffect()
{
  ForceNetMsg_StopPlaneEffect m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startOneHandDrawing()
{
  ForceNetMsg_OneHandDrawing m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startMovingPlaneDrawing()
{
  ForceNetMsg_MovingPlaneDrawing m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::setViscousGain(float gain)
{
  ForceNetMsg_ViscousEffect m(gain);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::turnFrictionOn()
{
  ForceNetMsg_FrictionEffect m(1);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::turnFrictionOff()
{
  ForceNetMsg_FrictionEffect m(0);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::setGeometry(int id, RenderDevice::Primitive type, const Array<Vector3> &verts)
{
  ForceNetMsg_SetGeometry m(id, type, verts);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::clearGeometry(int id)
{
  ForceNetMsg_ClearGeometry m(id);
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::startHybridDrawing()
{
  ForceNetMsg_HybridDrawing m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::toggleDynamicDragging()
{
  ForceNetMsg_DynamicDraggingCurv m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::hybridDrawingTransition()
{
  ForceNetMsg_HybridDrawingTransition m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::playSound(const std::string &filename)
{
  ForceNetMsg_PlaySound m(filename);
  _conduit->send(m.type(), m);
}


void
ForceNetInterface::shutdown()
{
  ForceNetMsg_Shutdown m;
  _conduit->send(m.type(), m);
}

void
ForceNetInterface::pollForInput(Array<VRG3D::EventRef> &events)
{
  while (_conduit->waitingMessageType() != NO_MSG) {
    switch (_conduit->waitingMessageType()) {
              
    case EVENTNETMSG_TYPE: 
      {
        EventNetMsg mEvent;
        _conduit->receive(mEvent);
        events.append(mEvent.event);
        break;
      }            
    case EVENTBUFFERNETMSG_TYPE:
      {
        EventBufferNetMsg mEventBuf;
        _conduit->receive(mEventBuf);
        events.append(mEventBuf.eventBuffer);
      }
      break;

    case NO_MSG:
      break;
      
    default: 
      cout << "Received a message of unknown type, ignoring it." << endl;
      break;
    }
  }
}

} // end namespace
*/