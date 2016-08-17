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

#ifndef __INET_RECIPIENTBLOCKACKHANDLER_H
#define __INET_RECIPIENTBLOCKACKHANDLER_H

#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

/*
 * This class implements 9.21.3 Data and acknowledgment transfer using
 * immediate Block Ack policy and delayed Block Ack policy
 *
 * TODO: RecipientBlockAckAgreementProcedure ?
 */
class INET_API RecipientBlockAckAgreementHandler
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;

        std::map<std::pair<MACAddress, Tid>, RecipientBlockAckAgreement *> blockAckAgreements;

    protected:
        virtual void terminateAgreement(MACAddress originatorAddr, Tid tid);
        virtual RecipientBlockAckAgreement* addAgreement(Ieee80211AddbaRequest *addbaReq);
        virtual void updateAgreement(Ieee80211AddbaResponse *frame);
        virtual Ieee80211AddbaResponse* buildAddbaResponse(Ieee80211AddbaRequest *frame, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy);

    public:
        virtual RecipientBlockAckAgreement* getAgreement(Tid tid, MACAddress originatorAddr);
        virtual Ieee80211Delba* buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode); // FIXME
        virtual void processTransmittedAddbaResp(Ieee80211AddbaResponse *addbaResp);
        virtual void processReceivedAddbaRequest(Ieee80211AddbaRequest *addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback);
        virtual void processReceivedDelba(Ieee80211Delba *delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy);
        virtual void processTransmittedDelba(Ieee80211Delba *delba);
};

} // namespace ieee80211
} // namespace inet

#endif
