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

#include "OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

//
// The inactivity timer at the originator is reset when a BlockAck frame
// corresponding to the TID for which the Block Ack policy is set is received.
//
void OriginatorBlockAckAgreement::blockAckFrameReceived()
{
    scheduleInactivityTimer();
}


//
// Every STA shall maintain an inactivity timer for every negotiated Block Ack setup.
//
void OriginatorBlockAckAgreement::scheduleInactivityTimer()
{
    if (blockAckTimeoutValue != 0) {
        agreementHandler->cancelEvent(inactivityTimer);
        agreementHandler->scheduleAt(simTime() + blockAckTimeoutValue, inactivityTimer);
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
