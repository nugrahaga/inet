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

#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "RecipientAckPolicy.h"

namespace inet {
namespace ieee80211 {

void RecipientAckPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
    }
}

simtime_t RecipientAckPolicy::computeAckDuration(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame) const
{
    return rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame)->getDuration(dataOrMgmtFrame->getBitLength());
}

//
// The cases when an ACK frame can be generated are shown in the frame exchange sequences listed in
// Annex G. On receipt of a management frame of subtype Action NoAck, a STA shall not send an ACK frame
// in response.
//
bool RecipientAckPolicy::isAckNeeded(Ieee80211DataOrMgmtFrame* frame) const
{
    // TODO: add mgmt NoAck check
    return !frame->getTransmitterAddress().isMulticast();
}

//
// 8.3.1.4 ACK frame format
//
// For ACK frames sent by non-QoS STAs, if the More Fragments bit was equal to 0 in the Frame Control field
// of the immediately previous individually addressed data or management frame, the duration value is set to 0.
// In other ACK frames sent by non-QoS STAs, the duration value is the value obtained from the Duration/ID
// field of the immediately previous data, management, PS-Poll, BlockAckReq, or BlockAck*** frame minus the
// time, in microseconds, required to transmit the ACK frame and its SIFS interval. If the calculated duration
// includes a fractional microsecond, that value is rounded up to the next higher integer.
//
// NOTE: ** BlockAckReq, BlockAck to a NonQoS STA???
//
simtime_t RecipientAckPolicy::computeAckDurationField(Ieee80211DataOrMgmtFrame* frame) const
{
    return frame->getMoreFragments() ? ceil(frame->getDuration() - modeSet->getSifsTime() - computeAckDuration(frame)) : 0;
}

} /* namespace ieee80211 */
} /* namespace inet */
