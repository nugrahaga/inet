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

void RecipientAckProcedure::initialize(int stage) {
}

void RecipientAckProcedure::processReceivedFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame, IRecipientAckPolicy *ackPolicy, IProcedureCallback *callback)
{
    numReceivedAckableFrame++;
    // After a successful reception of a frame requiring acknowledgment, transmission of the ACK frame
    // shall commence after a SIFS period, without regard to the busy/idle state of the medium. (See Figure 9-9.)
    if (ackPolicy->isAckNeeded(dataOrMgmtFrame)) {
        auto ackFrame = recipientAckProcedure->buildAck(dataOrMgmtFrame);
        ackFrame->setDuration(ackPolicy->computeAckDurationField(dataOrMgmtFrame));
        callback->transmitControlResponseFrame(ackFrame, dataOrMgmtFrame, modeSet->getSifs());
        processTransmittedAck(ackFrame); // FIXME: too early
    }
}

void RecipientAckProcedure::processTransmittedAck(Ieee80211ACKFrame* ack)
{
    numSentAck++;
}

Ieee80211ACKFrame* RecipientAckProcedure::buildAck(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    Ieee80211ACKFrame *ack = new Ieee80211ACKFrame("ACK");
    ack->setReceiverAddress(dataOrMgmtFrame->getTransmitterAddress());
    return ack;
}

void RecipientAckProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
