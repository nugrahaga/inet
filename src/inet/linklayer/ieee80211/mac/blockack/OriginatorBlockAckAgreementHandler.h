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
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorBlockAckAgreementHandler : public cSimpleModule, public cListener
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;
        std::map<std::pair<MACAddress, Tid>, OriginatorBlockAckAgreement *> blockAckAgreements;

    protected:
        void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

    public:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module does not handle msgs"); }

        virtual OriginatorBlockAckAgreement *getAgreement(MACAddress receiverAddr, Tid tid);

        virtual Ieee80211AddbaRequest *buildAddbaRequest(MACAddress receiverAddr, Tid tid, int startingSequenceNumber, bool aMsduSupported, simtime_t blockAckTimeoutValue, int maximumAllowedBufferSize, bool delayedBlockAckPolicySupported);
        virtual Ieee80211Delba *buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode);
        virtual simtime_t getAddbaRequestTimeout() const;

        virtual void createAgreement(Ieee80211AddbaRequest *addbaRequest);
        virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, Ieee80211AddbaResponse *addbaResp);
        virtual void terminateAgreement(MACAddress originatorAddr, Tid tid);
};

} // namespace ieee80211
} // namespace inet

#endif
