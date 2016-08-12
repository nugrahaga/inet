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

#ifndef __INET_FRAMESEQUENCECONTEXT_H
#define __INET_FRAMESEQUENCECONTEXT_H

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/contract/IFrameSequence.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API FrameSequenceContext
{
    protected:
        InProgressFrames *inProgressFrames = nullptr;
        OriginatorAckProcedure *ackProcedure = nullptr;
        RtsProcedure *rtsProcedure = nullptr;
        TxopProcedure *txopProcedure = nullptr;
        OriginatorBlockAckProcedure *blockAckProcedure = nullptr;
        OriginatorBlockAckAgreementHandler *blockAckAgreementHandler = nullptr;
        OriginatorQoSAckPolicy *originatorAckPolicy = nullptr;

        const IIeee80211Mode *mode = nullptr;

        std::vector<IFrameSequenceStep *> steps;

    public:
        FrameSequenceContext(InProgressFrames *inProgressFrames, OriginatorAckProcedure *ackProcedure, RtsProcedure *rtsProcedure, TxopProcedure *txopProcedure, OriginatorBlockAckProcedure *blockAckProcedure, OriginatorBlockAckAgreementHandler *agreementHandler, OriginatorQoSAckPolicy *ackPolicy, const IIeee80211Mode *mode);
        virtual ~FrameSequenceContext();

        virtual void addStep(IFrameSequenceStep *step) { steps.push_back(step); }
        virtual int getNumSteps() const { return steps.size(); }
        virtual IFrameSequenceStep *getStep(int i) const { return steps[i]; }
        virtual IFrameSequenceStep *getLastStep() const { return steps.size() > 0 ? steps.back() : nullptr; }
        virtual IFrameSequenceStep *getStepBeforeLast() const { return steps.size() > 1 ? steps[steps.size() - 2] : nullptr; }

        OriginatorAckProcedure* getAckProcedure() { return ackProcedure; }
        InProgressFrames* getInProgressFrames() { return inProgressFrames; }
        RtsProcedure* getRtsProcedure() { return rtsProcedure; }
        TxopProcedure* getTxopProcedure() { return txopProcedure; }
        OriginatorBlockAckProcedure* getBlockAckProcedure() { return blockAckProcedure; }
        OriginatorBlockAckAgreementHandler *getBlockAckAgreementHandler() { return blockAckAgreementHandler; }
        OriginatorQoSAckPolicy *getAckPolicyProcedure() { return originatorAckPolicy; }
        const IIeee80211Mode* getMode() { return mode; }
};

} // namespace ieee80211
} // namespace inet

#endif
