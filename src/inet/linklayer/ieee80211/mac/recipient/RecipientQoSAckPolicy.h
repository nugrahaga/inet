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

#ifndef __INET_RECIPIENTQOSACKPOLICY_H
#define __INET_RECIPIENTQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/IQoSRateSelection.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

//
// The cases when an ACK frame can be generated are shown in the frame exchange sequences listed in
// Annex G.
//
class INET_API RecipientQoSAckPolicy : public cSimpleModule, public cListener
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;
        IQoSRateSelection *rateSelection = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

        simtime_t computeBasicBlockAckDuration(Ieee80211BlockAckReq* blockAckReq) const;
        simtime_t computeAckDuration(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const;

    public:
        virtual bool isAckNeeded(Ieee80211DataOrMgmtFrame* frame) const;
        virtual bool isBlockAckNeeded(Ieee80211BlockAckReq *blockAckReq) const;

        virtual simtime_t computeAckDurationField(Ieee80211DataOrMgmtFrame *frame) const;
        virtual simtime_t computeBasicBlockAckDurationField(Ieee80211BasicBlockAckReq *basicBlockAckReq) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECIPIENTQOSACKPOLICY_H
