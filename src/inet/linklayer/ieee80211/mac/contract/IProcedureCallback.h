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

#ifndef __INET_IPROCEDURECALLBACK_H
#define __INET_IPROCEDURECALLBACK_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IProcedureCallback
{
    public:
        virtual ~IProcedureCallback() { }

        virtual void transmitControlResponseFrame(Ieee80211Frame* responseFrame, Ieee80211Frame* receivedFrame) = 0;
        virtual void processMgmtFrame(Ieee80211ManagementFrame *mgmtFrame) = 0;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif /* INET_LINKLAYER_IEEE80211_MAC_CONTRACT_IPROCEDURECALLBACK_H_ */