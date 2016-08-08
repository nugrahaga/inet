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

#ifndef __INET_BASICRATESELECTION_H
#define __INET_BASICRATESELECTION_H

#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

//
// TODO: 9.7.6.5 Rate selection for control response frames
//
class INET_API BasicRateSelection : public IRateSelection, public cSimpleModule, public cListener
{
    protected:
        const Ieee80211ModeSet *modeSet = nullptr;
        const IIeee80211Mode *dataFrameMode = nullptr;  // only if rateControl == nullptr
        const IIeee80211Mode *multicastFrameMode = nullptr;
        const IIeee80211Mode *controlFrameMode = nullptr;
        const IIeee80211Mode *slowestMandatoryMode = nullptr;
        IRateControl *rateControl = nullptr;  // optional

    protected:
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    public:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void setRateControl(IRateControl *rateControl) override;
        virtual const IIeee80211Mode *getSlowestMandatoryMode() override;
        virtual const IIeee80211Mode *getModeForUnicastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) override;
        virtual const IIeee80211Mode *getModeForMulticastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) override;
        virtual const IIeee80211Mode *getModeForControlFrame(Ieee80211Frame *controlFrame) override;
        virtual const IIeee80211Mode* getResponseControlFrameMode() override;
        virtual const Ieee80211ModeSet *getModeSet() { return modeSet; }

};

} // namespace ieee80211
} // namespace inet

#endif
