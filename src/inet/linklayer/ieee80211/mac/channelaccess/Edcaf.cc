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
//

#include "Edcaf.h"

namespace inet {
namespace ieee80211 {

Define_Module(Edcaf);

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

void Edcaf::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        ac = getAccessCategory(par("accessCategory"));
        contention = check_and_cast<IContention *>(getSubmodule("contention"));
        collisionController = check_and_cast<IEdcaCollisionController *>(getModuleByPath(par("collisionControllerModule")));
        auto rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        rx->registerContention(contention);
        auto rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        auto referenceMode = rateSelection->getSlowestMandatoryMode();
        slotTime = referenceMode->getSlotTime();
        sifs = referenceMode->getSifsTime();
        int aifsn = par("aifsn");
        simtime_t aifs = sifs + fallback(getAifsNumber(ac), aifsn) * slotTime;
        ifs = aifs;
        eifs = sifs + aifs + referenceMode->getDuration(LENGTH_ACK);
        ASSERT(ifs > sifs);
        cwMin = par("cwMin");
        cwMax = par("cwMax");
        if (cwMin == -1)
            cwMin = getCwMin(ac, referenceMode->getLegacyCwMin());
        if (cwMax == -1)
            cwMax = getCwMax(ac, referenceMode->getLegacyCwMax(), referenceMode->getLegacyCwMin());
        cw = cwMin;
    }
}

void Edcaf::incrementCw()
{
    int newCw = 2 * cw + 1;
    if (newCw > cwMax)
        cw = cwMax;
    else
        cw = newCw;
}

void Edcaf::resetCw()
{
    cw = cwMin;
}

int Edcaf::getAifsNumber(AccessCategory ac)
{
    switch (ac)
    {
        case AC_BK: return 7;
        case AC_BE: return 3;
        case AC_VI: return 2;
        case AC_VO: return 2;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

AccessCategory Edcaf::getAccessCategory(const char *ac)
{
    if (strcmp("AC_BK", ac) == 0)
        return AC_BK;
    if (strcmp("AC_VI", ac) == 0)
        return AC_VI;
    if (strcmp("AC_VO", ac) == 0)
        return AC_VO;
    if (strcmp("AC_BE", ac) == 0)
        return AC_BE;
    throw cRuntimeError("Unknown Access Category = %s", ac);
}

void Edcaf::channelAccessGranted()
{
    ASSERT(callback != nullptr);
    if (!collisionController->isInternalCollision(this)) {
        owning = true;
        callback->channelGranted(this);
    }
    contentionInProgress = false;
}

void Edcaf::releaseChannel(IChannelAccess::ICallback* callback)
{
    ASSERT(owning);
    owning = false;
    contentionInProgress = false;
    this->callback = nullptr;
}

void Edcaf::requestChannel(IChannelAccess::ICallback* callback)
{
    this->callback = callback;
    if (owning)
        callback->channelGranted(this);
    else if (!contentionInProgress) {
        contentionInProgress = true;
        contention->startContention(cw, ifs, eifs, slotTime, this);
    }
    else ;
}

void Edcaf::expectedChannelAccess(simtime_t time)
{
    collisionController->expectedChannelAccess(this, time);
}

bool Edcaf::isInternalCollision()
{
    return collisionController->isInternalCollision(this);
}

int Edcaf::getCwMax(AccessCategory ac, int aCwMax, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMax;
        case AC_BE: return aCwMax;
        case AC_VI: return aCwMin;
        case AC_VO: return (aCwMin + 1) / 2 - 1;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

int Edcaf::getCwMin(AccessCategory ac, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMin;
        case AC_BE: return aCwMin;
        case AC_VI: return (aCwMin + 1) / 2 - 1;
        case AC_VO: return (aCwMin + 1) / 4 - 1;
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}


} // namespace ieee80211
} // namespace inet
