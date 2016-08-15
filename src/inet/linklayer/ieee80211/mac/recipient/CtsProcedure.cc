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

CtsProcedure::CtsProcedure(IRx *rx, IRateSelection *rateSelection) :
    rx(rx),
    rateSelection(rateSelection)
{
}

void CtsProcedure::processReceivedRts(Ieee80211RTSFrame* rtsFrame)
{
    // don't care
}

simtime_t CtsProcedure::getCtsDuration(Ieee80211RTSFrame *rtsFrame) const
{
    return rateSelection->computeResponseCtsFrameMode(rtsFrame)->getDuration(LENGTH_CTS);
}

simtime_t CtsProcedure::getTimeout() const
{
    return modeSet->getSifsTime() + modeSet->getSlotTime() + modeSet->getPhyRxStartDelay();
}

//
// A STA that is addressed by an RTS frame shall transmit a CTS frame after a SIFS period if the NAV at the
// STA receiving the RTS frame indicates that the medium is idle. If the NAV at the STA receiving the RTS
// indicates the medium is not idle, that STA shall not respond to the RTS frame. The RA field of the CTS frame
// shall be the value obtained from the TA field of the RTS frame to which this CTS frame is a response. The
// Duration field in the CTS frame shall be the duration field from the received RTS frame, adjusted by
// subtraction of aSIFSTime and the number of microseconds required to transmit the CTS frame at a data rate
// determined by the rules in 9.7.
//
Ieee80211CTSFrame *CtsProcedure::buildCts(Ieee80211RTSFrame* rtsFrame)
{
    if (rx->isMediumFree()) {
        Ieee80211CTSFrame *cts = new Ieee80211CTSFrame("CTS");
        cts->setReceiverAddress(rtsFrame->getTransmitterAddress());
        cts->setDuration(ceil(rtsFrame->getDuration() - modeSet->getSifsTime() - getCtsDuration(rtsFrame)));
        return cts;
    }
    else
        return nullptr;
}

void CtsProcedure::processTransmittedCts(Ieee80211CTSFrame* ctsFrame)
{
    delete ctsFrame;
}

void CtsProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
