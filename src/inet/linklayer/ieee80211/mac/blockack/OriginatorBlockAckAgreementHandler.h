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

#ifndef __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements...
 * TODO: OriginatorBlockAckAgreementProcedure?
 */
class INET_API OriginatorBlockAckAgreementHandler
{
    protected:
        std::map<std::pair<MACAddress, Tid>, OriginatorBlockAckAgreement *> blockAckAgreements;

    protected:
        virtual Ieee80211AddbaRequest *buildAddbaRequest(MACAddress receiverAddr, Tid tid, int startingSequenceNumber, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy);
        virtual void createAgreement(Ieee80211AddbaRequest *addbaRequest);
        virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, Ieee80211AddbaResponse *addbaResp);
        virtual void terminateAgreement(MACAddress originatorAddr, Tid tid);

    public:
        virtual OriginatorBlockAckAgreement *getAgreement(MACAddress receiverAddr, Tid tid);
        virtual Ieee80211Delba *buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode); // FIXME:
        virtual void processReceivedBlockAck(Ieee80211BlockAck *blockAck, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);
        virtual void processTransmittedAddbaReq(Ieee80211AddbaRequest *addbaReq);
        virtual void processTransmittedDataFrame(Ieee80211DataFrame *dataFrame, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback);
        virtual void processReceivedAddbaResp(Ieee80211AddbaResponse *addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback);
        virtual void processReceivedDelba(Ieee80211Delba *delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);
        virtual void processTransmittedDelba(Ieee80211Delba *delba);
};

} // namespace ieee80211
} // namespace inet

#endif
