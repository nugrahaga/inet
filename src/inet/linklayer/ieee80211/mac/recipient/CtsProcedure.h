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

#ifndef __INET_CTSPROCEDURE_H
#define __INET_CTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"


namespace inet {
namespace ieee80211 {

class INET_API CtsProcedure
{
    protected:
        int numReceivedRts = 0;
        int numSentCts = 0;

    public:
        Ieee80211CTSFrame *buildCts(Ieee80211RTSFrame *rtsFrame);

        void processReceivedRts(Ieee80211RTSFrame *rtsFrame);
        void processTransmittedCts(Ieee80211CTSFrame *ctsFrame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_CTSPROCEDURE_H
