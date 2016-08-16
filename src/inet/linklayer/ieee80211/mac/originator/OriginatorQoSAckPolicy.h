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

#ifndef __INET_ORIGINATORQOSACKPOLICY_H
#define __INET_ORIGINATORQOSACKPOLICY_H

#include  "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class INET_API OriginatorQoSAckPolicy : public cSimpleModule, public cListener
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;

        int maxBlockAckPolicyFrameLength = -1;
        int blockAckReqTreshold = -1;

        simtime_t blockAckTimeout = -1;
        simtime_t ackTimeout = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize() override;

        virtual bool checkAgreementPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement);
        std::map<MACAddress, std::vector<Ieee80211DataFrame*>> getOutstandingFramesPerReceiver(InProgressFrames *inProgressFrames);
        virtual int computeStartingSequenceNumber(const std::vector<Ieee80211DataFrame*>& outstandingFrames);
        virtual bool isCompressedBlockAckReq(const std::vector<Ieee80211DataFrame*>& outstandingFrames, int startingSequenceNumber);

    public:
        virtual AckPolicy getAckPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement);
        virtual bool isBlockAckPolicyEligibleFrame(Ieee80211DataFrame* frame);
        virtual std::tuple<MACAddress, SequenceNumber, Tid> computeBlockAckReqParameters(InProgressFrames *inProgressFrames);
        virtual bool isBlockAckReqNeeded(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure);

        virtual simtime_t getAckTimeout(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const;
        virtual simtime_t getBlockAckTimeout(Ieee80211BlockAckReq *blockAckReq) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORQOSACKPOLICY_H
