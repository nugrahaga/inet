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

#include "RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

RecipientAckProcedure::RecipientAckProcedure(IRateSelection *rateSelection) :
    rateSelection(rateSelection)
{
   this->sifs = rateSelection->getSlowestMandatoryMode()->getSifsTime();
   this->slotTime = rateSelection->getSlowestMandatoryMode()->getSlotTime();
   this->phyRxStartDelay = rateSelection->getSlowestMandatoryMode()->getPhyRxStartDelay();
}

void RecipientAckProcedure::processReceivedFrame(Ieee80211Frame* frame)
{
}

void RecipientAckProcedure::processTransmittedAck(Ieee80211ACKFrame* ack)
{
    delete ack;
}

bool RecipientAckProcedure::isAckNeeded(Ieee80211Frame* frame)
{
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        if (dataFrame->getAckPolicy() != NORMAL_ACK)
            return false;
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame)) {
        if (dataOrMgmtFrame->getTransmitterAddress().isMulticast())
            return false;
        return true;
    }
    return false;
}

simtime_t RecipientAckProcedure::getAckEarlyTimeout() const
{
    // Note: This excludes ACK duration. If there's no RXStart indication within this interval, retransmission should begin immediately
    return sifs + slotTime + phyRxStartDelay;
}


simtime_t RecipientAckProcedure::getAckDuration() const
{
    return rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK);
}

simtime_t RecipientAckProcedure::getAckFullTimeout() const
{
    return sifs + slotTime + getAckDuration();
}

Ieee80211ACKFrame* RecipientAckProcedure::buildAck(Ieee80211Frame* frame)
{
    Ieee80211ACKFrame *ack = new Ieee80211ACKFrame("ACK");
    ack->setReceiverAddress(dataOrMgmtFrame->getTransmitterAddress());
    if (!frame->getMoreFragments())
        ack->setDuration(0);
    else {
        // For an ACK frame, the Duration/ID field is set to the value obtained from the Duration/ID field of the frame
        // that elicited the response minus the time, in microseconds between the end of the PPDU carrying the frame
        // that elicited the response and the end of the PPDU carrying the ACK frame.
        // TODO: slotTime
        ack->setDuration(frame->getDuration() - sifs - rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK));
    }
    return ack;
}

} /* namespace ieee80211 */
} /* namespace inet */

