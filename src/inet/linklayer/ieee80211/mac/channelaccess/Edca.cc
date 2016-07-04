//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "Edca.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

namespace inet {
namespace ieee80211 {

Define_Module(Edca);

void Edca::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        numEdcafs = par("numEdcafs");
        edcafs = new Edcaf*[numEdcafs];
        for (int ac = 0; ac < numEdcafs; ac++) {
            edcafs[ac] = check_and_cast<Edcaf*>(getSubmodule("edcaf", ac));
        }
    }
}

Tid Edca::getTid(Ieee80211DataOrMgmtFrame* frame)
{
    if (auto addbaResponse = dynamic_cast<Ieee80211AddbaResponse*>(frame)) {
        return addbaResponse->getTid();
    }
    else if (auto qosDataFrame = dynamic_cast<Ieee80211DataFrame*>(frame)) {
        ASSERT(qosDataFrame->getType() == ST_DATA_WITH_QOS);
        return qosDataFrame->getTid();
    }
    else
        throw cRuntimeError("Unknown frame type = %d", frame->getType());
}

AccessCategory Edca::classifyFrame(Ieee80211DataOrMgmtFrame *frame)
{
    return mapTidToAc(getTid(frame));
}

int Edca::getCwMax(AccessCategory ac, int aCwMax, int aCwMin)
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

int Edca::getCwMin(AccessCategory ac, int aCwMin)
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

AccessCategory Edca::mapTidToAc(Tid tid)
{
    // standard static mapping (see "UP-to-AC mappings" table in the 802.11 spec.)
    switch (tid) {
        case 1: case 2: return AC_BK;
        case 0: case 3: return AC_BE;
        case 4: case 5: return AC_VI;
        case 6: case 7: return AC_VO;
        default: throw cRuntimeError("No mapping from TID=%d to AccessCategory (must be in the range 0..7)", tid);
    }
}

Edcaf* Edca::getChannelOwner()
{
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isOwning())
            return edcafs[ac];
    return nullptr;
}

std::vector<Edcaf*> Edca::getInternallyCollidedEdcafs()
{
    std::vector<Edcaf*> collidedEdcafs;
    for (int ac = 0; ac < numEdcafs; ac++)
        if (edcafs[ac]->isInternalCollision())
            collidedEdcafs.push_back(edcafs[ac]);
    return collidedEdcafs;
}

void Edca::requestChannelAccess(AccessCategory ac, IContentionBasedChannelAccess::ICallback* callback)
{
    edcafs[ac]->requestChannel(callback);
}

void Edca::releaseChannelAccess(AccessCategory ac, IContentionBasedChannelAccess::ICallback* callback)
{
    edcafs[ac]->releaseChannel(callback);
}

} // namespace ieee80211
} // namespace inet

