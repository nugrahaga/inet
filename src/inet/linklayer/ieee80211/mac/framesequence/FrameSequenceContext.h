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
#include "inet/linklayer/ieee80211/mac/contract/IRtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IQoSRtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorQoSAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/OriginatorAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/originator/RtsProcedure.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API QoSContext
{
    public:
        RtsProcedure *rtsProcedure = nullptr;
        IQoSRtsPolicy *rtsPolicy = nullptr;
        OriginatorQoSAckPolicy *ackPolicy = nullptr; // TODO: interface
        OriginatorBlockAckProcedure *blockAckProcedure = nullptr; // TODO: interface
        OriginatorBlockAckAgreementHandler *blockAckAgreementHandler = nullptr;
        TxopProcedure *txopProcedure = nullptr;
};

class INET_API NonQoSContext
{
    public:
        RtsProcedure *rtsProcedure = nullptr;  // TODO: interface
        IRtsPolicy *rtsPolicy = nullptr;
        OriginatorAckPolicy *ackPolicy = nullptr; // TODO: interface
};

class INET_API FrameSequenceContext
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;
        InProgressFrames *inProgressFrames = nullptr;
        std::vector<IFrameSequenceStep *> steps;

        NonQoSContext *nonQoSContext = nullptr;
        QoSContext *qosContext = nullptr;

    public:
        FrameSequenceContext(Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, NonQoSContext *nonQosContext, QoSContext *qosContext);
        virtual ~FrameSequenceContext();

        virtual void addStep(IFrameSequenceStep *step) { steps.push_back(step); }
        virtual int getNumSteps() const { return steps.size(); }
        virtual IFrameSequenceStep *getStep(int i) const { return steps[i]; }
        virtual IFrameSequenceStep *getLastStep() const { return steps.size() > 0 ? steps.back() : nullptr; }
        virtual IFrameSequenceStep *getStepBeforeLast() const { return steps.size() > 1 ? steps[steps.size() - 2] : nullptr; }

        virtual InProgressFrames* getInProgressFrames() const { return inProgressFrames; }
        virtual simtime_t getIfs() const { return getNumSteps() == 0 ? 0 : modeSet->getSifsTime(); }

        virtual NonQoSContext *getNonQoSContext() const { return nonQoSContext; }
        virtual QoSContext *getQoSContext() const { return qosContext; }
};

} // namespace ieee80211
} // namespace inet

#endif
