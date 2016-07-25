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

package inet.linklayer.ieee80211.mac.coordinationfunction;

import inet.linklayer.ieee80211.mac.channelaccess.Dcaf;
import inet.linklayer.ieee80211.mac.framesequence.FrameSequenceHandler;
import inet.linklayer.ieee80211.mac.originator.OriginatorMacDataService;
import inet.linklayer.ieee80211.mac.originator.RecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.RtsProcedure;
import inet.linklayer.ieee80211.mac.recipient.RecipientMacDataService;


//
// IEEE 802.11 Distributed Coordination Function
//
module Dcf
{
    parameters:
        int maxQueueSize = default(100);

        string rxModule;
        string txModule;
        string rateSelectionModule;

        *.rateSelectionModule = "^." + rateSelectionModule;
        *.rxModule = "^." + rxModule;

        @class(Dcf);
        @display("i=block/queue;bgb=226,514");

    submodules:
        channelAccess: Dcaf {
            @display("p=113,293");
        }
        recipientMacDataService: RecipientMacDataService {
            @display("p=113,442");
        }
        originatorMacDataService: OriginatorMacDataService {
            @display("p=113,366");
        }
        frameSequenceHandler: FrameSequenceHandler {
            @display("p=113,211");
        }
        rtsProcedure: RtsProcedure {
            @display("p=113,57");
        }
        recoveryProcedure: RecoveryProcedure {
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=113,131");
        }
}
