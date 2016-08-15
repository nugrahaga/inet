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

#ifndef __INET_CTSPOLICY_H
#define __INET_CTSPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class INET_API CtsPolicy : public cSimpleModule, public cListener
{
    protected:
        IRx *rx = nullptr;
        IRateSelection *rateSelection = nullptr;
        Ieee80211ModeSet *modeSet = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

        simtime_t getCtsDuration(Ieee80211RTSFrame *rtsFrame) const;

    public:
        virtual bool isCtsNeeded(Ieee80211RTSFrame *rtsFrame) const;
        virtual simtime_t computeCtsDurationField(Ieee80211RTSFrame *frame) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_CTSPOLICY_H
