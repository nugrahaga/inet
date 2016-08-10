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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

CtsProcedure::CtsProcedure(IRx *rx, IRateSelection *rateSelection) :
    rx(rx),
    rateSelection(rateSelection)
{
    this->sifs = rateSelection->getSlowestMandatoryMode()->getSifsTime();
    this->slotTime = rateSelection->getSlowestMandatoryMode()->getSlotTime();
    this->phyRxStartDelay = rateSelection->getSlowestMandatoryMode()->getPhyRxStartDelay();
}

// TODO: move this part to somewhere else
Ieee80211Frame *CtsProcedure::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
    return frame;
}

void CtsProcedure::processReceivedRts(Ieee80211RTSFrame* rtsFrame)
{
    // don't care
}

simtime_t CtsProcedure::getCtsDuration() const
{
    return rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS);
}

simtime_t CtsProcedure::getCtsEarlyTimeout() const
{
    return sifs + slotTime + phyRxStartDelay; // see getAckEarlyTimeout()
}

simtime_t CtsProcedure::getCtsFullTimeout() const
{
    return sifs + slotTime + getCtsDuration();
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
        cts->setDuration(rtsFrame->getDuration() - sifs - rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS));
        setFrameMode(cts, rateSelection->getModeForControlFrame(cts));
        return cts;
    }
    else
        return nullptr;
}

void CtsProcedure::processTransmittedCts(Ieee80211CTSFrame* ctsFrame)
{
    delete ctsFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */
