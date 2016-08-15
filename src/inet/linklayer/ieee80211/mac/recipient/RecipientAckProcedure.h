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

#ifndef __INET_RECIPIENTACKPROCEDURE_H
#define __INET_RECIPIENTACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// 9.3.2.8 ACK procedure
//
// The cases when an ACK frame can be generated are shown in the frame exchange sequences listed in
// Annex G.
// On receipt of a management frame of subtype Action NoAck, a STA shall not send an ACK frame in response.
// Upon successful reception of a frame of a type that requires acknowledgment with the To DS field set, an AP
// shall generate an ACK frame. An ACK frame shall be transmitted by the destination STA that is not an AP,
// when it successfully receives an individually addressed frame of a type that requires acknowledgment, but not
// if it receives a group addressed frame of such type. After a successful reception of a frame requiring
// acknowledgment, transmission of the ACK frame shall commence after a SIFS period, without regard to the
// busy/idle state of the medium. (See Figure 9-9.)
//
class INET_API RecipientAckProcedure : public cListener
{
    protected:
        IRateSelection *rateSelection = nullptr;
        Ieee80211ModeSet *modeSet = nullptr;

    protected:
        simtime_t getAckDuration(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const;
        void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

    public:
        virtual ~RecipientAckProcedure() { };
        RecipientAckProcedure(IRateSelection *rateSelection);

        static bool isAckNeeded(Ieee80211DataOrMgmtFrame *frame);

        virtual void processReceivedFrame(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame);
        virtual void processTransmittedAck(Ieee80211ACKFrame *ack);
        virtual Ieee80211ACKFrame* buildAck(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTACKPROCEDURE_H
