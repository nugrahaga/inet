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

#include "inet/common/NotifierConsts.h"
#include "RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

RecipientAckProcedure::RecipientAckProcedure(IRateSelection *rateSelection) :
    rateSelection(rateSelection)
{
    // TODO: modeSet signal
}

void RecipientAckProcedure::processReceivedFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
}

void RecipientAckProcedure::processTransmittedAck(Ieee80211ACKFrame* ack)
{
    delete ack;
}

// TODO: RecipientAckPolicy
bool RecipientAckProcedure::isAckNeeded(Ieee80211DataOrMgmtFrame* frame)
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


simtime_t RecipientAckProcedure::getAckDuration(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const
{
    return rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame)->getDuration(LENGTH_ACK);
}

Ieee80211ACKFrame* RecipientAckProcedure::buildAck(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    Ieee80211ACKFrame *ack = new Ieee80211ACKFrame("ACK");
    ack->setReceiverAddress(dataOrMgmtFrame->getTransmitterAddress());
    if (!dataOrMgmtFrame->getMoreFragments())
        ack->setDuration(0);
    else {
        // Non-QoS: For ACK frames sent by non-QoS STAs, if the More Fragments bit was equal to 0 in the Frame Control field
        // of the immediately previous individually addressed data or management frame, the duration value is set to 0.
        // In other ACK frames sent by non-QoS STAs, the duration value is the value obtained from the Duration/ID
        // field of the immediately previous data, management, PS-Poll, BlockAckReq, or BlockAck frame minus the
        // time, in microseconds, required to transmit the ACK frame and its SIFS interval. If the calculated duration
        // includes a fractional microsecond, that value is rounded up to the next higher integer.

        // QoS: For an ACK frame, the Duration/ID field is set to the value obtained from the Duration/ID field of the frame
        // that elicited the response minus the time, in microseconds between the end of the PPDU carrying the frame
        // that elicited the response and the end of the PPDU carrying the ACK frame.
        ack->setDuration(ceil(dataOrMgmtFrame->getDuration() - modeSet->getSifsTime() - getAckDuration(dataOrMgmtFrame)));
    }
    return ack;
}

void RecipientAckProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
