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

#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "RecipientQoSAckPolicy.h"

namespace inet {
namespace ieee80211 {

void RecipientQoSAckPolicy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(NF_MODESET_CHANGED, this);
    }
}

simtime_t RecipientQoSAckPolicy::computeBasicBlockAckDuration(Ieee80211BlockAckReq* blockAckReq) const
{
    return rateSelection->computeResponseBlockAckFrameMode(blockAckReq)->getDuration(LENGTH_BASIC_BLOCKACK);
}

simtime_t RecipientQoSAckPolicy::computeAckDuration(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame) const
{
    return rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame)->getDuration(dataOrMgmtFrame->getBitLength());
}


bool RecipientQoSAckPolicy::isAckNeeded(Ieee80211DataOrMgmtFrame* frame) const
{
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        if (dataFrame->getAckPolicy() != NORMAL_ACK)
            return false;
    return !dataOrMgmtFrame->getTransmitterAddress().isMulticast();
}

bool RecipientQoSAckPolicy::isBlockAckNeeded(Ieee80211BlockAckReq* blockAckReq) const
{
    if (auto basicBlockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq>(blockAckReq)) {
        Tid tid = basicBlockAckReq->getTidInfo();
        MACAddress originatorAddr = basicBlockAckReq->getTransmitterAddress();
        RecipientBlockAckAgreement *agreement = agreementHandler->getAgreement(tid, originatorAddr);
        return agreement != nullptr;
    }
    else
        throw cRuntimeError("Unsupported BlockAckReq");
}

//
// 8.2.5.7 Setting for control response frames
// For an ACK frame, the Duration/ID field is set to the value obtained from the Duration/ID field of the frame
// that elicited the response minus the time, in microseconds between the end of the PPDU carrying the frame
// that elicited the response and the end of the PPDU carrying the ACK frame.
//
simtime_t RecipientQoSAckPolicy::computeAckDurationField(Ieee80211DataOrMgmtFrame* frame) const
{
    return frame->getDuration() - modeSet->getSifsTime() - computeAckDuration(frame);
}

//
// For a BlockAck frame transmitted in response to a BlockAckReq frame or transmitted in response to a frame
// containing an implicit Block Ack request, the Duration/ID field is set to the value obtained from the
// Duration/ID field of the frame that elicited the response minus the time, in microseconds, between the end of
// the PPDU carrying the frame that elicited the response and the end of the PPDU carrying the BlockAck
// frame.
//
simtime_t RecipientQoSAckPolicy::computeBasicBlockAckDurationField(Ieee80211BasicBlockAckReq* basicBlockAckReq) const
{
    return blockAckReq->getDuration() - modeSet->getSifsTime() - computeBasicBlockAckDuration(basicBlockAckReq);
}

void RecipientQoSAckPolicy::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
