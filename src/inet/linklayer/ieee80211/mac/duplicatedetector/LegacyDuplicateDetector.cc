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

#include "inet/common/stlutils.h"
#include "inet/linklayer/ieee80211/mac/duplicatedetector/LegacyDuplicateDetector.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// A non-QoS STA shall assign sequence numbers to management frames and data frames (QoS subfield of the
// Subtype field is equal to 0) from a single modulo-4096 counter, starting at 0 and incrementing by 1, for each
// MSDU or MMPDU.
//
void LegacyDuplicateDetector::assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    lastSeqNum = (lastSeqNum + 1) % 4096;
    frame->setSequenceNumber(lastSeqNum);
}

bool LegacyDuplicateDetector::isDuplicate(Ieee80211DataOrMgmtFrame *frame)
{
    ASSERT(frame->getType() != ST_DATA_WITH_QOS);
    const MACAddress& address = frame->getTransmitterAddress();
    SequenceControlField seqVal(frame);
    auto it = lastSeenSeqNumCache.find(address);
    if (it == lastSeenSeqNumCache.end())
        lastSeenSeqNumCache.insert(std::pair<MACAddress, SequenceControlField>(address, seqVal));
    else if (it->second.getSequenceNumber() == seqVal.getSequenceNumber() && it->second.getFragmentNumber() == seqVal.getFragmentNumber() && frame->getRetry())
        return true;
    else
        it->second = seqVal;
    return false;
}

} // namespace ieee80211
} // namespace inet
