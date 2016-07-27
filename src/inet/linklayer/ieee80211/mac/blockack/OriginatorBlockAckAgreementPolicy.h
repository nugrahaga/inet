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

namespace inet {
namespace ieee80211 {

enum class BaPolicyAction {
    SEND_ADDBA_REQUEST,
    SEND_BA_REQUEST,
    SEND_WITH_BLOCK_ACK,
    SEND_WITH_NORMAL_ACK
};

class INET_API OriginatorBlockAckAgreementPolicy : public cSimpleModule
{
    protected:
        OriginatorBlockAckAgreementHandler *agreementHandler = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        BaPolicyAction getAckPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement);
        bool isEligibleFrame(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement);
        BaPolicyAction getAction(FrameSequenceContext *context);

    public:
        virtual void processUpperFrame(Ieee80211DataOrMgmtFrame *frame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_ORIGINATORBLOCKACKAGREEMENTPOLICY_H
