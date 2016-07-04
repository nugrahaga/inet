//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_RECOVERYPROCEDURE_H
#define __INET_RECOVERYPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// References: 9.19.2.6 Retransmit procedures (IEEE 802.11-2012 STD)
//             802.11 Reference Design: Recovery Procedures and Retransmit Limits
//             (https://warpproject.org/trac/wiki/802.11/MAC/Lower/Retransmissions)
// FIXME: per tid and mgmt frames or change the internal data structure
// Sequence numbers for management frames, QoS data frames with a group address in the Address 1 field,
// and all non-QoS data frames transmitted by QoS STAs shall be assigned using an additional single
// modulo-4096 counter, starting at 0 and incrementing by 1 for each such MSDU, A-MSDU, or MMPDU, except
// that a QoS STA may use values from additional modulo-4096 counters per <Address 1, TID> for sequence
// numbers assigned to time priority management frames.
class INET_API RecoveryProcedure : public cSimpleModule, public IRecoveryProcedure
{

    protected:
        std::map<SequenceNumber, int> shortRetryCounter; // SRC
        std::map<SequenceNumber, int> longRetryCounter; // LRC

        int stationLongRetryCounter = 0; // SLRC
        int stationShortRetryCounter = 0; // SSRC
        int cwMin = -1;
        int cwMax = -1;
        int cw = -1; // Contention window

        int shortRetryLimit = -1;
        int longRetryLimit = -1;
        int rtsThreshold = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        void incrementCounter(Ieee80211DataOrMgmtFrame* frame, std::map<SequenceNumber, int>& retryCounter);
        void incrementStationSrc();
        void incrementStationLrc();
        void resetStationSrc() { stationShortRetryCounter = 0; }
        void resetStationLrc() { stationLongRetryCounter = 0; }
        void resetContentionWindow() { cw = cwMin; }
        int doubleCw(int cw);
        int getRc(Ieee80211DataOrMgmtFrame* frame, std::map<SequenceNumber, int>& retryCounter);
        bool isMulticastFrame(Ieee80211Frame *frame);

    public:
        virtual void multicastFrameTransmitted();

        virtual void ctsFrameReceived();
        virtual void ackFrameReceived(Ieee80211DataOrMgmtFrame *ackedFrame);
        virtual void blockAckFrameReceived();

        virtual void rtsFrameTransmissionFailed(Ieee80211DataOrMgmtFrame *protectedFrame);
        virtual void dataOrMgmtFrameTransmissionFailed(Ieee80211DataOrMgmtFrame *failedFrame);

        virtual bool isDataOrMgtmFrameRetryLimitReached(Ieee80211DataOrMgmtFrame* failedFrame);
        virtual bool isRtsFrameRetryLimitReached(Ieee80211DataOrMgmtFrame* protectedFrame);

        virtual int getCw() override { return cw; }
        virtual int getCwMax() { return cwMax; }
        virtual int getCwMin() { return cwMin; }
        virtual int getLongRetryLimit() { return longRetryLimit; }
        virtual int getShortRetryLimit() { return shortRetryLimit; }

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECOVERYPROCEDURE_H
