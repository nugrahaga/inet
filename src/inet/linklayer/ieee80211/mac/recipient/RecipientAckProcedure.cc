//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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

Ieee80211ACKFrame* RecipientAckProcedure::buildAck(Ieee80211Frame* frame) {
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        if (dataFrame->getAckPolicy() != NORMAL_ACK)
            return nullptr;
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame)) {
        if (dataOrMgmtFrame->getTransmitterAddress().isMulticast())
            return nullptr;
        Ieee80211ACKFrame *ack = new Ieee80211ACKFrame("ACK");
        ack->setReceiverAddress(dataOrMgmtFrame->getTransmitterAddress());
        if (!frame->getMoreFragments())
            ack->setDuration(0);
        else
            ack->setDuration(frame->getDuration() - sifs - rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK));
        // setFrameMode(ack, rateSelection->getModeForControlFrame(ack)); // FIXME: move
        return ack;
    }
    return nullptr;
}

} /* namespace ieee80211 */
} /* namespace inet */

