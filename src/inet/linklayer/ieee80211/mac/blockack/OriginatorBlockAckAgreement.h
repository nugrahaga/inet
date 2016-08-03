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

#ifndef INET_ORIGINATORBLOCKACKAGREEMENT_H
#define INET_ORIGINATORBLOCKACKAGREEMENT_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreementHandler;

class OriginatorBlockAckAgreement
{
    protected:
        MACAddress receiverAddr = MACAddress::UNSPECIFIED_ADDRESS;
        Tid tid = -1;

        int numSentBaPolicyFrames = 0;
        SequenceNumber startingSequenceNumber = -1;
        int bufferSize = -1;
        bool isAMsduSupported = false;
        bool isDelayedBlockAckPolicySupported = false;
        bool isAddbaResponseReceived = false;
        simtime_t blockAckTimeoutValue = -1;
        cMessage *inactivityTimer = nullptr;

    public:
        OriginatorBlockAckAgreement(MACAddress receiverAddr, Tid tid, SequenceNumber startingSequenceNumber, int bufferSize, bool isAMsduSupported, bool isDelayedBlockAckPolicySupported) :
            receiverAddr(receiverAddr),
            tid(tid),
            startingSequenceNumber(startingSequenceNumber),
            bufferSize(bufferSize),
            isAMsduSupported(isAMsduSupported),
            isDelayedBlockAckPolicySupported(isDelayedBlockAckPolicySupported),
            inactivityTimer(new cMessage("InactivityTimer"))
        { }

        int getBufferSize() const { return bufferSize; }
        SequenceNumber getStartingSequenceNumber() { return startingSequenceNumber; }
        bool getIsAddbaResponseReceived() const { return isAddbaResponseReceived; }
        bool getIsAMsduSupported() const { return isAMsduSupported; }
        bool getIsDelayedBlockAckPolicySupported() const { return isDelayedBlockAckPolicySupported; }
        MACAddress getReceiverAddr() const { return receiverAddr; }
        Tid getTid() const { return tid; }
        const simtime_t getBlockAckTimeoutValue() const  { return blockAckTimeoutValue; }
        int getNumSentBaPolicyFrames() const { return numSentBaPolicyFrames; }

        void setBufferSize(int bufferSize) { this->bufferSize = bufferSize; }
        void setIsAddbaResponseReceived(bool isAddbaResponseReceived) { this->isAddbaResponseReceived = isAddbaResponseReceived; }
        void setIsAMsduSupported(bool isAMsduSupported) { this->isAMsduSupported = isAMsduSupported; }
        void setIsDelayedBlockAckPolicySupported(bool isDelayedBlockAckPolicySupported) { this->isDelayedBlockAckPolicySupported = isDelayedBlockAckPolicySupported; }
        void setBlockAckTimeoutValue(const simtime_t blockAckTimeoutValue) { this->blockAckTimeoutValue = blockAckTimeoutValue; }

        void baPolicyFrameSent() { numSentBaPolicyFrames++; }
        cMessage *getInactivityTimer() { return inactivityTimer; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORBLOCKACKAGREEMENT_H
