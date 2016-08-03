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
    AlternativesFs({new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isDataRtsCtsNeeded)),
                                      new DataFs(BLOCK_ACK)}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isDataRtsCtsNeeded)),
                                      new DataFs(NORMAL_ACK),
                                      new AckFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isBlockAckReqRtsCtsNeeded)),
                                      new BlockAckReqBlockAckFs()}),
                    new SequentialFs({new OptionalFs(new RtsCtsFs(), OPTIONALFS_PREDICATE(isMgmtRtsCtsNeeded)),
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
    Ieee80211Frame *frameToTransmit = context->getInProgressFrames()->getFrameToTransmit();
    if (dynamic_cast<Ieee80211ManagementFrame*>(frameToTransmit))
        return 3;
    else {
        Ieee80211DataFrame *dataFrameToTransmit = check_and_cast<Ieee80211DataFrame*>(frameToTransmit);
        auto agreement = context->getBlockAckAgreementHandler()->getAgreement(dataFrameToTransmit->getReceiverAddress(), dataFrameToTransmit->getTid());
        if (dynamic_cast<Ieee80211BlockAckReq*>(frameToTransmit))
            return 2;
        else if (agreement == nullptr)
            return 1;
        else if (context->getAckPolicy()->getAckPolicy(dataFrameToTransmit, agreement) == AckPolicy::BLOCK_ACK)
            return 0;
        else
            return 1;
    }
}

bool TxOpFs::isMgmtRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return context->getInProgressFrames()->getFrameToTransmit()->getByteLength() > context->getRtsProcedure()->getRtsThreshold();
}

bool TxOpFs::isDataRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return context->getInProgressFrames()->getFrameToTransmit()->getByteLength() > context->getRtsProcedure()->getRtsThreshold();
}

bool TxOpFs::isBlockAckReqRtsCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context)
{
    return false;
}

} // namespace ieee80211
} // namespace inet
