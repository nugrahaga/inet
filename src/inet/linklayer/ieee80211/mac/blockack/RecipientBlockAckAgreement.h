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

#ifndef __INET_RECIPIENTBLOCKACKAGREEMENT_H
#define __INET_RECIPIENTBLOCKACKAGREEMENT_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreement
{
    protected:
        BlockAckRecord *blockAckRecord = nullptr;

        SequenceNumber startingSequenceNumber = -1;
        int bufferSize = -1;
        simtime_t blockAckTimeoutValue = 0;

        bool isAddbaResponseSent = false;

    public:
        RecipientBlockAckAgreement(MACAddress originatorAddress, Tid tid, SequenceNumber startingSequenceNumber, int bufferSize, simtime_t blockAckTimeoutValue);

        virtual void blockAckPolicyFrameReceived(Ieee80211DataFrame *frame);

        virtual BlockAckRecord *getBlockAckRecord() const { return blockAckRecord; }
        virtual simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
        virtual int getBufferSize() const { return bufferSize; }
        virtual int getStartingSequenceNumber() const { return startingSequenceNumber; }

        virtual void addbaResposneSent() { isAddbaResponseSent = true; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTBLOCKACKAGREEMENT_H
