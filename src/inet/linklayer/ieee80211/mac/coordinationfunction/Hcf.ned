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

import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementHandler;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementHandler;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.framesequence.FrameSequenceHandler;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSMacDataService;
import inet.linklayer.ieee80211.mac.originator.RecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.RtsProcedure;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.recipient.RecipientQoSMacDataService;


//
// HCF -- Hybrid Coordination Function
//
module Hcf
{
    parameters:
        int numEdcafs = default(4);
        int maxQueueSize = default(100);
        string rateSelectionModule;
        string rxModule;
        string txModule;

        *.rateSelectionModule = "^." + rateSelectionModule;
        *.rxModule = "^." + rxModule;

        @class(Hcf);
        @display("i=block/queue;bgb=627,680");

    submodules:
        edca: Edca {
            numEdcafs = numEdcafs;
            @display("p=52,262");
        }
        hcca: Hcca {
            @display("p=52,348");
        }
        originatorQoSMacDataService: OriginatorQoSMacDataService {
            @display("p=483,527");
        }
        recipientQoSMacDataService: RecipientQoSMacDataService {
            recipientBlockAckAgreementHandlerModule = "^.recipientBlockAckAgreementHandler";
            @display("p=132,527");
        }
        frameSequenceHandler: FrameSequenceHandler {
            @display("p=122,65");
        }
        originatorBlockAckAgreementHandler: OriginatorBlockAckAgreementHandler {
            @display("p=483,605");
        }
        recipientBlockAckAgreementHandler: RecipientBlockAckAgreementHandler {
            @display("p=132,605");
        }
        rtsProcedure: RtsProcedure {
            @display("p=343,169");
        }
        edcaRecoveryProcedures[numEdcafs]: RecoveryProcedure {
            cwCalculator = "^";
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=343,342,row,50");
        }
        edcaTxopProcedures[numEdcafs]: TxopProcedure {
            @display("p=343,262,row,50");
        }
}
