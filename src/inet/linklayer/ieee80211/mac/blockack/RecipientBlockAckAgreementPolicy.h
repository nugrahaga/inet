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

#ifndef __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H
#define __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreementPolicy : public cSimpleModule, public IRecipientBlockAckAgreementPolicy
{
    protected:
        RecipientBlockAckAgreementHandler *agreementHandler = nullptr;
        std::map<std::pair<MACAddress, Tid>, cMessage *> inacitivityTimers;

        int maximumAllowedBufferSize = -1;
        bool isAMsduSupported = false;
        bool isDelayedBlockAckPolicySupported = false;
        simtime_t blockAckTimeoutValue = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;

        virtual void scheduleInactivityTimer(RecipientBlockAckAgreement *agreement);
        virtual std::pair<MACAddress, Tid> findAgreement(cMessage* inactivityTimer);

    public:
        ~RecipientBlockAckAgreementPolicy();

        virtual bool isAddbaReqAccepted(Ieee80211AddbaRequest* addbaReq) override;
        virtual bool isDelbaAccepted(Ieee80211Delba* delba) override;
        virtual void qosFrameReceived(Ieee80211DataFrame *qosFrame) override;

        virtual void agreementEstablished(RecipientBlockAckAgreement* agreement) override;

        virtual simtime_t getBlockAckTimeoutValue() const override { return blockAckTimeoutValue; }
        virtual bool aMsduSupported() const override { return isAMsduSupported; }
        virtual bool delayedBlockAckPolicySupported() const override { return isDelayedBlockAckPolicySupported; }
        virtual int getMaximumAllowedBufferSize() const override { return maximumAllowedBufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H