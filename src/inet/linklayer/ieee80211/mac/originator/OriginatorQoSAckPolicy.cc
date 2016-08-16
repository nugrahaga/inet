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

#include "OriginatorQoSAckPolicy.h"
#include <tuple>

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorQoSAckPolicy);

void OriginatorQoSAckPolicy::initialize()
{
    maxBlockAckPolicyFrameLength = par("maxBlockAckPolicyFrameLength");
    blockAckReqTreshold = par("blockAckReqTreshold");
    blockAckTimeout = par("blockAckTimeout");
    ackTimeout = par("ackTimeout");
}

std::map<MACAddress, std::vector<Ieee80211DataFrame*>> OriginatorQoSAckPolicy::getOutstandingFramesPerReceiver(InProgressFrames *inProgressFrames)
{
    auto outstandingFrames = inProgressFrames->getOutstandingFrames();
    std::map<MACAddress, std::vector<Ieee80211DataFrame*>> outstandingFramesPerReceiver;
    for (auto frame : outstandingFrames)
        outstandingFramesPerReceiver[frame->getReceiverAddress()].push_back(frame);
    return outstandingFramesPerReceiver;
}


int OriginatorQoSAckPolicy::computeStartingSequenceNumber(const std::vector<Ieee80211DataFrame*>& outstandingFrames)
{
    ASSERT(outstandingFrames.size() > 0);
    int startingSequenceNumber = outstandingFrames[0]->getSequenceNumber();
    for (int i = 1; i < (int)outstandingFrames.size(); i++) {
        int seqNum = outstandingFrames[i]->getSequenceNumber();
        if (seqNum < startingSequenceNumber)
            startingSequenceNumber = seqNum;
    }
    return startingSequenceNumber;
}

bool OriginatorQoSAckPolicy::isCompressedBlockAckReq(const std::vector<Ieee80211DataFrame*>& outstandingFrames, int startingSequenceNumber)
{
    // The Compressed Bitmap subfield of the BA Control field or BAR Control field shall be set to 1 in all
    // BlockAck and BlockAckReq frames sent from one HT STA to another HT STA and shall be set to 0 otherwise.
    return false; // non-HT STA
//    for (auto frame : outstandingFrames)
//        if (frame->getSequenceNumber() >= startingSequenceNumber && frame->getFragmentNumber() > 0)
//            return false;
//    return true;
}

bool OriginatorQoSAckPolicy::isBlockAckReqNeeded(InProgressFrames* inProgressFrames, TxopProcedure* txopProcedure)
{
    // FIXME
    simtime_t remainingTxop = txopProcedure->getRemaining();
    auto outstandingFramesPerReceiver = getOutstandingFramesPerReceiver(inProgressFrames);
    for (auto outstandingFrames : outstandingFramesPerReceiver) {
        if ((int)outstandingFrames.second.size() >= blockAckReqTreshold)
            return true;
    }
    return false;
}

std::tuple<MACAddress, SequenceNumber, Tid> OriginatorQoSAckPolicy::computeBlockAckReqParameters(InProgressFrames *inProgressFrames)
{
    auto outstandingFramesPerReceiver = getOutstandingFramesPerReceiver(inProgressFrames);
    for (auto outstandingFrames : outstandingFramesPerReceiver) {
        if ((int)outstandingFrames.second.size() >= blockAckReqTreshold) {
            auto largestOutstandingFrames = outstandingFramesPerReceiver.begin();
            for (auto it = outstandingFramesPerReceiver.begin(); it != outstandingFramesPerReceiver.end(); it++) {
                if (it->second.size() > largestOutstandingFrames->second.size())
                    largestOutstandingFrames = it;
            }
            MACAddress receiverAddress = largestOutstandingFrames->first;
            SequenceNumber startingSequenceNumber = computeStartingSequenceNumber(largestOutstandingFrames->second);
            Tid tid = largestOutstandingFrames->second.at(0)->getTid();
            return std::make_tuple(receiverAddress, startingSequenceNumber, tid);
        }
    }
    return std::make_tuple(MACAddress::UNSPECIFIED_ADDRESS, -1, -1);
}

AckPolicy OriginatorQoSAckPolicy::getAckPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement)
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

bool OriginatorQoSAckPolicy::isBlockAckPolicyEligibleFrame(Ieee80211DataFrame* frame)
{
    return frame->getByteLength() < maxBlockAckPolicyFrameLength;
}

bool OriginatorQoSAckPolicy::checkAgreementPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement)
{
    bool bufferFull = agreement->getBufferSize() == agreement->getNumSentBaPolicyFrames();
    bool aMsduOk = agreement->getIsAMsduSupported() || !frame->getAMsduPresent();
    // TODO: bool baPolicy = agreement->getIsDelayedBlockAckPolicySupported() || !frame->getAckPolicy();
    return !bufferFull && aMsduOk && (frame->getSequenceNumber() >= agreement->getStartingSequenceNumber()); // TODO: && baPolicy
}

//
// After transmitting an MPDU that requires an ACK frame as a response (see Annex G), the STA shall wait for an
// ACKTimeout interval, with a value of aSIFSTime + aSlotTime + aPHY-RX-START-Delay, starting at the
// PHY-TXEND.confirm primitive. If a PHY-RXSTART.indication primitive does not occur during the
// ACKTimeout interval, the STA concludes that the transmission of the MPDU has failed, and this STA shall
// invoke its backoff procedure upon expiration of the ACKTimeout interval.
//
simtime_t OriginatorQoSAckPolicy::getAckTimeout(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame) const
{
    if (ackTimeout == -1)
        return modeSet->getSifsTime() + modeSet->getSlotTime() + modeSet->getPhyRxStartDelay();
    return ackTimeout;
}

simtime_t OriginatorQoSAckPolicy::getBlockAckTimeout(Ieee80211BlockAckReq* blockAckReq) const
{
    if (blockAckTimeout == -1)
        return modeSet->getSifsTime() + modeSet->getSlotTime() + modeSet->getPhyRxStartDelay();
    return blockAckTimeout;
}

void OriginatorQoSAckPolicy::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
