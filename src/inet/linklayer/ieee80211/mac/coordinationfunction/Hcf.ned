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
import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementHandler;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.framesequence.FrameSequenceHandler;
import inet.linklayer.ieee80211.mac.originator.NonQoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSMacDataService;
import inet.linklayer.ieee80211.mac.originator.QoSRecoveryProcedure;
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
        @display("i=block/queue;bgb=752.39996,783.864");

    submodules:
        edca: Edca {
            numEdcafs = numEdcafs;
            @display("p=52,262");
        }
        hcca: Hcca {
            @display("p=52,348");
        }
        originatorQoSMacDataService: OriginatorQoSMacDataService {
            @display("p=552.672,526.68");
        }
        recipientQoSMacDataService: RecipientQoSMacDataService {
            recipientBlockAckAgreementHandlerModule = "^.recipientBlockAckAgreementHandler";
            @display("p=184.68,526.68");
        }
        frameSequenceHandler: FrameSequenceHandler {
            @display("p=122,65");
        }
        originatorBlockAckAgreementHandler: OriginatorBlockAckAgreementHandler {
            @display("p=552.672,618.336");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy {
            originatorBlockAckAgreementHandlerModule = "^.originatorBlockAckAgreementHandler";
            originatorAckPolicyModule = "^.originatorAckPolicy";
            @display("p=552.672,716.832");
        }
        recipientBlockAckAgreementHandler: RecipientBlockAckAgreementHandler {
            @display("p=184.68,618.336");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy {
            recipientBlockAckAgreementHandlerModule = "^.recipientBlockAckAgreementHandler";
            @display("p=184.68,716.832");
        }
        rtsProcedure: RtsProcedure {
            @display("p=366.624,117.647995");
        }
        edcaMgmtAndNonQoSRecoveryProcedure: NonQoSRecoveryProcedure {
            cwCalculatorModule = "^.edca.edcaf[1]"; // best effort
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=366.624,400.824");
        }
        edcaDataRecoveryProcedures[numEdcafs]: QoSRecoveryProcedure {
            cwCalculatorModule = "^.edca.edcaf[" + string(index) + "]";
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=366.624,305.064,row,50");
        }
        edcaTxopProcedures[numEdcafs]: TxopProcedure {
            @display("p=366.624,207.93599,row,50");
        }
        originatorAckPolicy: OriginatorAckPolicy {
            @display("p=581.39996,64.296");
        }
}
