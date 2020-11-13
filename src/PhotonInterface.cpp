#include "PhotonInterface.h"

namespace DrawOnAir {
	PhotonInterface::PhotonInterface(ArtworkRef artwork, EventMgrRef eventMgr, HistoryRef history) {
		_artwork = artwork;
		_eventMgr = eventMgr;
		_history = history;

		_fsa = new Fsa("Photon");
		_fsa->addState("Start");
		_fsa->addArc("NewMark", "Start", "Start", Array<std::string>("NewMark"));
		_fsa->addArcCallback("NewMark", this, &PhotonInterface::newMark);
		_eventMgr->addFsaRef(_fsa);
	}

	PhotonInterface::~PhotonInterface() {

	}

	void PhotonInterface::newMark(MinVR::EventRef e) {
		std::string message = e->getMsgData();
		G3D::BinaryInput b((const uint8*)message.c_str(), message.size(), G3DEndian::G3D_LITTLE_ENDIAN);
		//b.decompress();
		_artwork->deserializeMark(b);
	}
}