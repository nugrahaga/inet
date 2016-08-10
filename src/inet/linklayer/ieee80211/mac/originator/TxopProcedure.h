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

#ifndef __INET_TXOPPROCEDURE_H
#define __INET_TXOPPROCEDURE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

using namespace physicallayer;

class INET_API TxopProcedure : public cSimpleModule
{
    protected:
        IRateSelection *rateSelection = nullptr;
        simtime_t start = -1;
        simtime_t limit = -1;

    protected:
        s getTxopLimit(const IIeee80211Mode *mode, AccessCategory ac);

    public:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual void startTxop(AccessCategory ac);
        virtual void stopTxop();

        virtual simtime_t getStart() const;
        virtual simtime_t getLimit() const;
        virtual simtime_t getRemaining() const;

        virtual bool isFinalFragment(Ieee80211Frame *frame);
        virtual bool isTxopInitiator(Ieee80211Frame *frame);
        virtual bool isTxopTerminator(Ieee80211Frame *frame);
};

} /* namespace ieee80211 */
} /* namespace inet */


#endif // ifndef __INET_TXOPPROCEDURE_H
