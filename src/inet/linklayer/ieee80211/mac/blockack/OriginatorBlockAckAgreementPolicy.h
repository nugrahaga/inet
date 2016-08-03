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

#ifndef __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H
#define __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQoSAckPolicy.h"

namespace inet {
namespace ieee80211 {

//
// TODO: ADDBAFailureTimeout -- 6.3.29.2.2 Semantics of the service primitive
//
class INET_API OriginatorBlockAckAgreementPolicy : public cSimpleModule
{
    protected:
        OriginatorQoSAckPolicy *ackPolicy = nullptr;
        OriginatorBlockAckAgreementHandler *agreementHandler = nullptr;

        int blockAckReqTreshold = -1;
        bool delayedAckPolicySupported = false;
        bool aMsduSupported = false;
        int maximumAllowedBufferSize = -1;
        simtime_t blockAckTimeoutValue = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize() override;
        virtual void handleMessage(cMessage* msg) override;

        virtual void scheduleInactivityTimer(OriginatorBlockAckAgreement *agreement);

    public:
        virtual bool isAddbaReqNeeded(Ieee80211DataFrame *frame);
        virtual bool isAddbaReqAccepted(Ieee80211AddbaResponse *addbaResp, OriginatorBlockAckAgreement* agreement);
        virtual bool isDelbaAccepted(Ieee80211Delba *delba);

        virtual void blockAckReceived(OriginatorBlockAckAgreement *agreement);
        virtual void agreementEstablished(OriginatorBlockAckAgreement *agreement);

        virtual bool isMsduSupported() const { return aMsduSupported; }
        virtual simtime_t getBlockAckTimeoutValue() const { return blockAckTimeoutValue; }
        virtual bool isDelayedAckPolicySupported() const { return delayedAckPolicySupported; }
        virtual int getMaximumAllowedBufferSize() const { return maximumAllowedBufferSize; }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H
