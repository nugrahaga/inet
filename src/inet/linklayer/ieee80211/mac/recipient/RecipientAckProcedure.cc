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

#include "inet/common/NotifierConsts.h"
#include "RecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

void RecipientAckProcedure::processReceivedFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    numReceivedAckableFrame++;
}

void RecipientAckProcedure::processTransmittedAck(Ieee80211ACKFrame* ack)
{
    numSentAck++;
    delete ack;
}

Ieee80211ACKFrame* RecipientAckProcedure::buildAck(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    Ieee80211ACKFrame *ack = new Ieee80211ACKFrame("ACK");
    ack->setReceiverAddress(dataOrMgmtFrame->getTransmitterAddress());
    return ack;
}

} /* namespace ieee80211 */
} /* namespace inet */
