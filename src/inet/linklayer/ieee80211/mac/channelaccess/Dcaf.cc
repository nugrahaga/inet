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

#include "Dcaf.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(Dcaf);

void Dcaf::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        contention = check_and_cast<IContention *>(getSubmodule("contention"));
        auto rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        auto rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rx->registerContention(contention);
        auto referenceMode = rateSelection->getSlowestMandatoryMode();
        slotTime = referenceMode->getSlotTime();
        sifs = referenceMode->getSifsTime();
        int difs = par("difsTime");
        ifs = difs == -1 ? sifs + 2 * slotTime : difs;
        eifs = sifs + ifs + referenceMode->getDuration(LENGTH_ACK);
    }
}

void Dcaf::channelAccessGranted()
{
    ASSERT(callback != nullptr);
    callback->channelGranted(this);
    owning = true;
    contentionInProgress = false;
}

void Dcaf::releaseChannel(IContentionBasedChannelAccess::ICallback* callback)
{
    owning = false;
    contentionInProgress = false;
    this->callback = nullptr;
}

void Dcaf::requestChannel(IContentionBasedChannelAccess::ICallback* callback)
{
    this->callback = callback;
    if (owning)
        callback->channelGranted(this);
    else if (!contentionInProgress) {
        contention->startContention(callback->getCw(this), ifs, eifs, slotTime, this);
        contentionInProgress = true;
    }
    else ;
}

void Dcaf::expectedChannelAccess(simtime_t time)
{
    // don't care
}

} /* namespace ieee80211 */
} /* namespace inet */
