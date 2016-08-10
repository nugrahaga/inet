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

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

OriginatorBlockAckProcedure::OriginatorBlockAckProcedure(IQoSRateSelection* rateSelection) :
    rateSelection(rateSelection)
{
}

Ieee80211BlockAckReq* OriginatorBlockAckProcedure::buildCompressedBlockAckReqFrame(const MACAddress& receiverAddress, Tid tid, int startingSequenceNumber) const
{
    throw cRuntimeError("Unsupported feature");
    Ieee80211CompressedBlockAckReq *blockAckReq = new Ieee80211CompressedBlockAckReq("CompressedBlockAckReq");
    blockAckReq->setReceiverAddress(receiverAddress);
    // FIXME: blockAckReq->setDuration();
    blockAckReq->setStartingSequenceNumber(startingSequenceNumber);
    blockAckReq->setTidInfo(tid);
    return blockAckReq;
}

Ieee80211BlockAckReq* OriginatorBlockAckProcedure::buildBasicBlockAckReqFrame(const MACAddress& receiverAddress, Tid tid, int startingSequenceNumber) const
{
    Ieee80211BasicBlockAckReq *blockAckReq = new Ieee80211BasicBlockAckReq("BasicBlockAckReq");
    blockAckReq->setReceiverAddress(receiverAddress);
    blockAckReq->setStartingSequenceNumber(startingSequenceNumber);
    blockAckReq->setTidInfo(tid);
    return blockAckReq;
}

simtime_t OriginatorBlockAckProcedure::getBlockAckEarlyTimeout(Ieee80211BlockAckReq* blockAckReq) const
{
    auto mode = rateSelection->computeResponseBlockAckFrameMode(blockAckReq);
    return mode->getSifsTime() + mode->getSlotTime() + mode->getPhyRxStartDelay();
}

simtime_t OriginatorBlockAckProcedure::getBlockAckFullTimeout(Ieee80211BlockAckReq* blockAckReq) const
{
    if (dynamic_cast<Ieee80211BasicBlockAckReq*>(blockAckReq)) {
        auto mode = rateSelection->computeResponseBlockAckFrameMode(blockAckReq);
        return mode->getSifsTime() + mode->getSlotTime() + mode->getDuration(LENGTH_BASIC_BLOCKACK);
    }
    else
        throw cRuntimeError("Unsupported BlockAck Request");
}

} /* namespace ieee80211 */
} /* namespace inet */
