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

#include "RtsProcedure.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(RtsProcedure);

void RtsProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        this->rtsThreshold = par("rtsThreshold");
        // TODO: control bitrate
        rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
        this->sifs = rateSelection->getSlowestMandatoryMode()->getSifsTime();
        this->slotTime = rateSelection->getSlowestMandatoryMode()->getSlotTime();
        this->phyRxStartDelay = rateSelection->getSlowestMandatoryMode()->getPhyRxStartDelay();
    }
}

Ieee80211RTSFrame *RtsProcedure::buildRtsFrame(Ieee80211DataOrMgmtFrame *dataFrame) const
{
    const IIeee80211Mode *mode = dataFrame->getControlInfo() ? getFrameMode(dataFrame) : nullptr;
    if (isBroadcastOrMulticast(dataFrame))
        mode = rateSelection->getModeForMulticastDataOrMgmtFrame(dataFrame);
    else
        mode = rateSelection->getModeForUnicastDataOrMgmtFrame(dataFrame);
    return buildRtsFrame(dataFrame, mode);
}

Ieee80211RTSFrame *RtsProcedure::buildRtsFrame(Ieee80211DataOrMgmtFrame *dataFrame, const IIeee80211Mode *dataFrameMode) const
{
    // protect CTS + Data + ACK
    // TODO: single protection mechanism takes care
    simtime_t duration =
            3 * sifs
            + rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS)
            + dataFrameMode->getDuration(dataFrame->getBitLength())
            + rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK);
    return buildRtsFrame(dataFrame->getReceiverAddress(), duration);
}

Ieee80211RTSFrame *RtsProcedure::buildRtsFrame(const MACAddress& receiverAddress, simtime_t duration) const
{
    Ieee80211RTSFrame *rts = new Ieee80211RTSFrame("RTS");
    rts->setReceiverAddress(receiverAddress);
    rts->setDuration(duration);
    setFrameMode(rts, rateSelection->getModeForControlFrame(rts));
    return rts;
}

simtime_t RtsProcedure::getCtsDuration() const
{
    return rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS);
}

//
// After transmitting an RTS frame, the STA shall wait for a CTSTimeout interval, with a value of aSIFSTime +
// aSlotTime + aPHY-RX-START-Delay, starting at the PHY-TXEND.confirm primitive. If a PHY-RXSTART.indication
// primitive does not occur during the CTSTimeout interval, the STA shall conclude that the transmission of
// the RTS has failed, and this STA shall invoke its backoff procedure upon expiration of the CTSTimeout interval.
//
simtime_t RtsProcedure::getCtsTimeout() const
{
    return sifs + slotTime + phyRxStartDelay;
}

bool RtsProcedure::isBroadcastOrMulticast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isMulticast();  // also true for broadcast frames
}

const IIeee80211Mode *RtsProcedure::getFrameMode(Ieee80211Frame *frame) const
{
    Ieee80211TransmissionRequest *ctrl = check_and_cast<Ieee80211TransmissionRequest*>(frame->getControlInfo());
    return ctrl->getMode();
}

// TODO: move this part to somewhere else
Ieee80211Frame *RtsProcedure::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
    return frame;
}

} /* namespace ieee80211 */
} /* namespace inet */
