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

#ifndef __INET_RECIPIENTBLOCKACKPROCEDURE_H
#define __INET_RECIPIENTBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

//
// 9.3.2.9 BlockAck procedure
//
// Upon successful reception of a frame of a type that requires an immediate BlockAck response, the receiving
// STA shall transmit a BlockAck frame after a SIFS period, without regard to the busy/idle state of the medium.
// The rules that specify the contents of this BlockAck frame are defined in 9.21.
//
class INET_API RecipientBlockAckProcedure
{
    protected:
        RecipientBlockAckAgreementHandler *agreementHandler = nullptr;

    public:
        RecipientBlockAckProcedure(RecipientBlockAckAgreementHandler *agreementHandler, IRateSelection *rateSelection);

        virtual void processReceivedBlockAckReq(Ieee80211BlockAckReq *blockAckReq);
        virtual void processTransmittedBlockAck(Ieee80211BlockAck *blockAck);
        virtual Ieee80211BlockAck* buildBlockAck(Ieee80211BlockAckReq *blockAckReq);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTBLOCKACKPROCEDURE_H
