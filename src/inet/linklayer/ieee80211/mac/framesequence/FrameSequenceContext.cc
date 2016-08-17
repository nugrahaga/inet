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

#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"

namespace inet {
namespace ieee80211 {

FrameSequenceContext::FrameSequenceContext(Ieee80211ModeSet *modeSet, InProgressFrames *inProgressFrames, NonQoSContext *nonQoSContext, QoSContext *qosContext) :
    modeSet(modeSet),
    inProgressFrames(inProgressFrames),
    nonQoSContext(nonQoSContext),
    qosContext(qosContext)
{
}

simtime_t FrameSequenceContext::getAckTimeout(Ieee80211DataOrMgmtFrame* dataOrMgmtframe) const
{
    return qosContext ? qosContext->ackPolicy->getAckTimeout(dataOrMgmtframe) : nonQoSContext->ackPolicy->getAckTimeout(dataOrMgmtframe);
}

simtime_t FrameSequenceContext::getCtsTimeout(Ieee80211RTSFrame* rtsFrame) const
{
    return qosContext ? qosContext->rtsPolicy->getCtsTimeout(rtsFrame) : nonQoSContext->rtsPolicy->getCtsTimeout(rtsFrame);

}

FrameSequenceContext::~FrameSequenceContext()
{
    for (auto step : steps)
        delete step;
}

} // namespace ieee80211
} // namespace inet
