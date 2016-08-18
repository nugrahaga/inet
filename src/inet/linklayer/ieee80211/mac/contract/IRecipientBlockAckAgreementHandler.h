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

#ifndef __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H
#define __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientBlockAckAgreementHandler
{
    public:
        virtual ~IRecipientBlockAckAgreementHandler() { };

        virtual RecipientBlockAckAgreement* getAgreement(Tid tid, MACAddress originatorAddr) = 0;
        virtual Ieee80211Delba* buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode) = 0;
        virtual void processTransmittedAddbaResp(Ieee80211AddbaResponse *addbaResp) = 0;
        virtual void processReceivedAddbaRequest(Ieee80211AddbaRequest *addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) = 0;
        virtual void processReceivedDelba(Ieee80211Delba *delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy) = 0;
        virtual void processTransmittedDelba(Ieee80211Delba *delba) = 0;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H
