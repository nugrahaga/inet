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

#ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
#define __INET_ORIGINATORPROTECTIONMECHANISM_H

#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorProtectionMechanism
{
    protected:
        IRateSelection *rateSelection = nullptr;

        simtime_t sifs = -1;

    protected:
        virtual simtime_t computeRtsDurationPerId(Ieee80211RTSFrame *rtsFrame, Ieee80211Frame *pendingFrame);
        virtual simtime_t computeDataFrameDurationPerId(Ieee80211DataFrame *dataFrame, Ieee80211Frame *pendingFrame);
        virtual simtime_t computeMgmtFrameDurationPerId(Ieee80211ManagementFrame *mgmtFrame, Ieee80211Frame *pendingFrame);

    public:
        virtual simtime_t computeDurationPerId(Ieee80211Frame *frame, Ieee80211Frame *pendingFrame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
