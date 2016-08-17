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

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"
#include "inet/linklayer/ieee80211/mac/framesequence/TxOpFs.h"

namespace inet {
namespace ieee80211 {

TxOpFs::TxOpFs() :
    // Excerpt from G.3 EDCA and HCCA sequences
    // txop-sequence =
    //   ( ( ( RTS CTS ) | CTS + self ) Data + individual + QoS +( block-ack | no-ack ) ) |
    //   [ RTS CTS ] (txop-part-requiring-ack txop-part-providing-ack )|
    //   [ RTS CTS ] (Management | ( Data + QAP )) + individual ACK |
    //   [ RTS CTS ] (BlockAckReq BlockAck ) |
    //   ht-txop-sequence;
    AlternativesFs({new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new DataFs(BLOCK_ACK)}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new DataFs(NORMAL_ACK),
                                      new AckFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isBlockAckReqRtsCtsNeeded)),
                                      new BlockAckReqBlockAckFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isRtsCtsNeeded)),
                                      new AlternativesFs({new ManagementFs(),
                                                          /* TODO: DATA + QAP*/},
                                                         ALTERNATIVESFS_SELECTOR(selectMgmtOrDataQap))})},
                   ALTERNATIVESFS_SELECTOR(selectTxOpSequence))
{
}

int TxOpFs::selectMgmtOrDataQap(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return 0;
}

int TxOpFs::selectTxOpSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    auto frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    if (context->getQoSContext()->ackPolicy->isBlockAckReqNeeded(context->getInProgressFrames(), context->getQoSContext()->txopProcedure))
        return 2;
    if (dynamic_cast<Ieee80211ManagementFrame*>(frameToTransmit))
        return 3;
    else {
        auto dataFrameToTransmit = check_and_cast<Ieee80211DataFrame*>(frameToTransmit);
        auto agreement = context->getQoSContext()->blockAckAgreementHandler->getAgreement(dataFrameToTransmit->getReceiverAddress(), dataFrameToTransmit->getTid());
        auto ackPolicy = context->getQoSContext()->ackPolicy->computeAckPolicy(dataFrameToTransmit, agreement);
        if (ackPolicy == AckPolicy::BLOCK_ACK)
            return 0;
        else if (ackPolicy == AckPolicy::NORMAL_ACK)
            return 1;
        else
            throw cRuntimeError("Unknown AckPolicy");
    }
}

bool TxOpFs::isRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    auto protectedFrame = context->getInProgressFrames()->getFrameToTransmit();
    auto txop = context->getQoSContext()->txopProcedure;
    return context->getQoSContext()->rtsPolicy->isRtsNeeded(protectedFrame, txop);
}

bool TxOpFs::isBlockAckReqRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return false; // FIXME: IQoSRtsPolicy should handle this case
}

} // namespace ieee80211
} // namespace inet
