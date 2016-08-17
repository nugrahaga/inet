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
#include "inet/linklayer/ieee80211/mac/recipient/RecipientQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.9 BlockAck procedure
 */
class INET_API RecipientBlockAckProcedure : public cSimpleModule, public cListener
{
    protected:
        RecipientBlockAckAgreementHandler *agreementHandler = nullptr;
        int numReceivedBlockAckReq = 0;
        int numSentBlockAck = 0;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize() override;

        virtual Ieee80211BlockAck* buildBlockAck(Ieee80211BlockAckReq *blockAckReq);

    public:
        virtual void processReceivedBlockAckReq(Ieee80211BlockAckReq *blockAckReq, IRecipientQoSAckPolicy *ackPolicy, IProcedureCallback *callback);
        virtual void processTransmittedBlockAck(Ieee80211BlockAck *blockAck);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTBLOCKACKPROCEDURE_H
