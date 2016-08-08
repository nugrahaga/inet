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

#include "OriginatorProtectionMechanism.h"

namespace inet {
namespace ieee80211 {

//
// For all RTS frames sent by non-QoS STAs, the duration value is the time, in microseconds, required to
// transmit the pending data or management frame, plus one CTS frame, plus one ACK frame, plus three SIFS
// intervals. If the calculated duration includes a fractional microsecond, that value is rounded up to the next
// higher integer. For RTS frames sent by QoS STAs, see 8.2.5.
//
simtime_t OriginatorProtectionMechanism::computeRtsDurationPerId(Ieee80211RTSFrame* rtsFrame, Ieee80211Frame* pendingFrame)
{
    simtime_t pendingFrameDuration = rateSelection->getMode(pendingFrame)->getDuration(pendingFrame->getBitLength());
    simtime_t ctsFrameDuration = rateSelection->getResponseCtsFrameMode()->getDuration(LENGTH_CTS);
    simtime_t ackFrameDuration = rateSelection->getResponseAckDuration()->getDuration(LENGTH_ACK);
    simtime_t durationId = ctsFrameDuration + pendingFrameDuration + ackFrameDuration;
    return durationId + 3 * sifs;
}

simtime_t OriginatorProtectionMechanism::computeDurationPerId(Ieee80211Frame* frame, Ieee80211Frame* pendingFrame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        return computeRtsDurationPerId(rtsFrame, pendingFrame);
}

} /* namespace ieee80211 */
} /* namespace inet */

