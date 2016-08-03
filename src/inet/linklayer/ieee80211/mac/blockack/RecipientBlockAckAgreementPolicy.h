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

namespace inet {
namespace ieee80211 {

class INET_API RecipientBlockAckAgreementPolicy : public cSimpleModule
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
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;

        virtual void scheduleInactivityTimer(RecipientBlockAckAgreement *agreement);
        virtual std::pair<MACAddress, Tid> findAgreement(cMessage* inactivityTimer);

    public:
        ~RecipientBlockAckAgreementPolicy();

        virtual bool isAddbaReqAccepted(Ieee80211AddbaRequest* addbaReq);
        virtual bool isDelbaAccepted(Ieee80211Delba* delba);
        virtual void qosFrameReceived(Ieee80211DataFrame *qosFrame);

        virtual void agreementEstablished(RecipientBlockAckAgreement* agreement);

        simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
        bool aMsduSupported() const { return isAMsduSupported; }
        bool delayedBlockAckPolicySupported() const { return isDelayedBlockAckPolicySupported; }
        int getMaximumAllowedBufferSize() const { return maximumAllowedBufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECIPIENTBLOCKACKAGREEMENTPOLICY_H
