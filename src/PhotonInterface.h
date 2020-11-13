/**
 * \author Ben Knorlein
 *
 * \file  PhotonInterface.H
 * \brief
 *	Interface to react on Photon messages
 */

#ifndef PHOTONINTERFACE_H
#define PHOTONINTERFACE_H

#include <EventMgr.h>
#include <Artwork.h>
#include <History.h>

namespace DrawOnAir {

	typedef G3D::ReferenceCountedPointer<class PhotonInterface> PhotonInterfaceRef;

	class PhotonInterface
	{
	public:
		PhotonInterface(ArtworkRef artwork, EventMgrRef eventMgr, HistoryRef history);
		~PhotonInterface();

		void newMark(MinVR::EventRef e);

	private:
		ArtworkRef         _artwork;
		EventMgrRef        _eventMgr;
		FsaRef             _fsa;
		HistoryRef         _history;
	};
} // end namespace

#endif PHOTONINTERFACE_H