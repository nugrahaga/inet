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

#include "CtsProcedure.h"
#include "inet/common/NotifierConsts.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

void CtsProcedure::processReceivedRts(Ieee80211RTSFrame* rtsFrame)
{
    numReceivedRts++;
}

Ieee80211CTSFrame *CtsProcedure::buildCts(Ieee80211RTSFrame* rtsFrame)
{
    Ieee80211CTSFrame *cts = new Ieee80211CTSFrame("CTS");
    // The RA field of the CTS frame shall be the value
    // obtained from the TA field of the to which this
    // CTS frame is a response.
    cts->setReceiverAddress(rtsFrame->getTransmitterAddress());
    return cts;
}

void CtsProcedure::processTransmittedCts(Ieee80211CTSFrame* ctsFrame)
{
    numSentCts++;
    delete ctsFrame;
}

} /* namespace ieee80211 */
} /* namespace inet */
