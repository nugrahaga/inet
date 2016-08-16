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

Define_Module(RecipientBlockAckProcedure);

void RecipientBlockAckProcedure::initialize()
{
    agreementHandler = check_and_cast<RecipientBlockAckAgreementHandler*>(getModuleByPath(par("blockAckAgreementHandlerModule")));
}

//
// Upon successful reception of a frame of a type that requires an immediate BlockAck response, the receiving
// STA shall transmit a BlockAck frame after a SIFS period, without regard to the busy/idle state of the medium.
// The rules that specify the contents of this BlockAck frame are defined in 9.21.
//
void RecipientBlockAckProcedure::processReceivedBlockAckReq(Ieee80211BlockAckReq* blockAckReq, IRecipientQoSAckPolicy *ackPolicy, IProcedureCallback *callback)
{
    numReceivedBlockAckReq++;
    if (ackPolicy->isBlockAckNeeded(blockAckRequest)) {
        auto blockAck = buildBlockAck(blockAckRequest);
        blockAck->setDuration(ackPolicy->computeBasicBlockAckDurationField(blockAckRequest));
        callback->transmitControlResponseFrame(blockAck, modeSet->getSifs(), this);
        processTransmittedBlockAck(blockAck); // FIXME: too early
    }
}

void RecipientBlockAckProcedure::processTransmittedBlockAck(Ieee80211BlockAck* blockAck)
{
    numSentBlockAck++;
    delete blockAck;
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
        ASSERT(agreement != nullptr);
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
        blockAck->setCompressedBitmap(false);
        blockAck->setStartingSequenceNumber(basicBlockAckReq->getStartingSequenceNumber());
        blockAck->setTidInfo(tid);
        return blockAck;
    }
    else
        throw cRuntimeError("Unsupported Block Ack Request");
}

void RecipientBlockAckProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}


} /* namespace ieee80211 */
} /* namespace inet */
