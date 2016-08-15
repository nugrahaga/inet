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
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "RecipientBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

RecipientBlockAckProcedure::RecipientBlockAckProcedure(RecipientBlockAckAgreementHandler* agreementHandler, IQoSRateSelection *rateSelection) :
        rateSelection(rateSelection),
        agreementHandler(agreementHandler)
{
}


void RecipientBlockAckProcedure::processReceivedBlockAckReq(Ieee80211BlockAckReq* blockAckReq)
{
}

void RecipientBlockAckProcedure::processTransmittedBlockAck(Ieee80211BlockAck* blockAck)
{
}

simtime_t RecipientBlockAckProcedure::computeBasicBlockAckDuration(Ieee80211BlockAckReq* blockAckReq) const
{
    return rateSelection->computeResponseBlockAckFrameMode(blockAckReq)->getDuration(LENGTH_BASIC_BLOCKACK);
}

//
// The Basic BlockAck frame contains acknowledgments for the MPDUs of up to 64 previous MSDUs. In the
// Basic BlockAck frame, the STA acknowledges only the MPDUs starting from the starting sequence control
// until the MPDU with the highest sequence number that has been received, and the STA shall set bits in the
// Block Ack bitmap corresponding to all other MPDUs to 0.
//
Ieee80211BlockAck* RecipientBlockAckProcedure::buildBlockAck(Ieee80211BlockAckReq* blockAckReq)
{
    if (auto basicBlockAckReq = dynamic_cast<Ieee80211BasicBlockAckReq*>(blockAckReq)) {
        Tid tid = basicBlockAckReq->getTidInfo();
        MACAddress originatorAddr = basicBlockAckReq->getTransmitterAddress();
        RecipientBlockAckAgreement *agreement = agreementHandler->getAgreement(tid, originatorAddr);
        if (agreement) {
            Ieee80211BasicBlockAck *blockAck = new Ieee80211BasicBlockAck("BasicBlockAck");
            int startingSequenceNumber = basicBlockAckReq->getStartingSequenceNumber();
            for (SequenceNumber seqNum = startingSequenceNumber; seqNum < startingSequenceNumber + 64; seqNum++) {
                BitVector &bitmap = blockAck->getBlockAckBitmap(seqNum - startingSequenceNumber);
                for (FragmentNumber fragNum = 0; fragNum < 16; fragNum++) {
                    bool ackState = agreement->getBlockAckRecord()->getAckState(seqNum, fragNum);
                    bitmap.setBit(fragNum, ackState);
                }
            }
            blockAck->setReceiverAddress(blockAckReq->getTransmitterAddress());
            // For a BlockAck frame transmitted in response to a BlockAckReq frame or transmitted in response to a frame
            // containing an implicit Block Ack request, the Duration/ID field is set to the value obtained from the
            // Duration/ID field of the frame that elicited the response minus the time, in microseconds, between the end of
            // the PPDU carrying the frame that elicited the response and the end of the PPDU carrying the BlockAck
            // frame.
            blockAck->setDuration(blockAckReq->getDuration() - modeSet->getSifsTime() - computeBasicBlockAckDuration(blockAckReq));
            blockAck->setCompressedBitmap(false);
            blockAck->setStartingSequenceNumber(basicBlockAckReq->getStartingSequenceNumber());
            blockAck->setTidInfo(tid);
            return blockAck;
        }
        else
            return nullptr;
    }
    else
        throw cRuntimeError("Unsupported Block Ack Request");
}

void RecipientBlockAckProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    //Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} /* namespace ieee80211 */
} /* namespace inet */
