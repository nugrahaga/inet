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

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "OriginatorAckPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorAckPolicy);

void OriginatorAckPolicy::initialize()
{
    maxBlockAckPolicyFrameLength = par("maxBlockAckPolicyFrameLength");
    blockAckReqTreshold = par("blockAckReqTreshold");
}

bool OriginatorAckPolicy::isBaReqNeeded(FrameSequenceContext *context)
{
    auto outstandingFramesPerReceiver = context->getOutstandingFramesPerReceiver();
    for (auto outstandingFrames : outstandingFramesPerReceiver) {
        if ((int)outstandingFrames.second.size() >= blockAckReqTreshold)
            return true;
    }
    return false;
}

AckPolicy OriginatorAckPolicy::getAckPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement)
{
    ASSERT(agreement);
    if (agreement->getIsAddbaResponseReceived() && isBlockAckPolicyEligibleFrame(frame)) {
        if (checkAgreementPolicy(frame, agreement))
            return AckPolicy::BLOCK_ACK;
        else
            return AckPolicy::NORMAL_ACK;
    }
    else
        return AckPolicy::NORMAL_ACK;
}

bool OriginatorAckPolicy::isBlockAckPolicyEligibleFrame(Ieee80211DataFrame* frame)
{
    return frame->getByteLength() < maxBlockAckPolicyFrameLength;
}

bool OriginatorAckPolicy::checkAgreementPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement)
{
    bool bufferFull = agreement->getBufferSize() == agreement->getNumSentBaPolicyFrames();
    bool aMsduOk = agreement->getIsAMsduSupported() || !frame->getAMsduPresent();
    // TODO: bool baPolicy = agreement->getIsDelayedBlockAckPolicySupported() || !frame->getAckPolicy();
    return !bufferFull && aMsduOk && (frame->getSequenceNumber() >= agreement->getStartingSequenceNumber()); // TODO: && baPolicy
}

} /* namespace ieee80211 */
} /* namespace inet */
