//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "CtsProcedure.h"
#include "inet/common/NotifierConsts.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

void CtsProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(NF_MODESET_CHANGED, this);
    }
}

void CtsProcedure::processReceivedRts(Ieee80211RTSFrame* rtsFrame, ICtsPolicy *ctsPolicy, IProcedureCallback *callback)
{
    numReceivedRts++;
    // A STA that is addressed by an RTS frame shall transmit a CTS frame after a SIFS period
    // if the NAV at the STA receiving the RTS frame indicates that the medium is idle.
    if (ctsPolicy->isCtsNeeded(rtsFrame)) {
        auto ctsFrame = buildCts(rtsFrame);
        ctsFrame->setDuration(ctsPolicy->computeCtsDurationField(rtsFrame));
        callback->transmitControlResponseFrame(ctsFrame, rtsFrame, modeSet->getSifsTime());
        processTransmittedCts(ctsFrame); // FIXME: too early
    }
    // If the NAV at the STA receiving the RTS indicates the medium is not idle,
    // that STA shall not respond to the RTS frame.
    else ;
}

Ieee80211CTSFrame *CtsProcedure::buildCts(Ieee80211RTSFrame* rtsFrame)
{
    Ieee80211CTSFrame *cts = new Ieee80211CTSFrame("CTS");
    // The RA field of the CTS frame shall be the value
    // obtained from the TA field of the to which this
    // CTS frame is a response.
    cts->setReceiverAddress(rtsFrame->getTransmitterAddress());
    return cts;
}

void CtsProcedure::processTransmittedCts(Ieee80211CTSFrame* ctsFrame)
{
    numSentCts++;
}

void CtsProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
