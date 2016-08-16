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

#ifndef __INET_ORIGINATORBLOCKACKPROCEDURE_H
#define __INET_ORIGINATORBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class INET_API OriginatorBlockAckProcedure : public cSimpleModule, public cListener
{
    protected:
        Ieee80211ModeSet *modeSet = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module does not handle msgs"); }
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details);

    public:
        virtual Ieee80211BlockAckReq *buildCompressedBlockAckReqFrame(const MACAddress& receiverAddress, Tid tid, int startingSequenceNumber) const;
        virtual Ieee80211BlockAckReq *buildBasicBlockAckReqFrame(const MACAddress& receiverAddress, Tid tid, int startingSequenceNumber) const;

        simtime_t getBlockAckReqTimeout(Ieee80211BlockAckReq* blockAckReq) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_ORIGINATORBLOCKACKPROCEDURE_H
