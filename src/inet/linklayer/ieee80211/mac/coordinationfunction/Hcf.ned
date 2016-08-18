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

import inet.linklayer.ieee80211.mac.blockack.OriginatorBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.blockack.RecipientBlockAckAgreementPolicy;
import inet.linklayer.ieee80211.mac.channelaccess.Edca;
import inet.linklayer.ieee80211.mac.channelaccess.Hcca;
import inet.linklayer.ieee80211.mac.contract.IRateControl;
import inet.linklayer.ieee80211.mac.originator.NonQoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSAckPolicy;
import inet.linklayer.ieee80211.mac.originator.OriginatorQoSMacDataService;
import inet.linklayer.ieee80211.mac.originator.QoSRecoveryProcedure;
import inet.linklayer.ieee80211.mac.originator.QoSRtsPolicy;
import inet.linklayer.ieee80211.mac.originator.TxopProcedure;
import inet.linklayer.ieee80211.mac.rateselection.QoSRateSelection;
import inet.linklayer.ieee80211.mac.recipient.QoSCtsPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQoSAckPolicy;
import inet.linklayer.ieee80211.mac.recipient.RecipientQoSMacDataService;


//
// HCF -- Hybrid Coordination Function
//
module Hcf
{
    parameters:
        int numEdcafs = default(4);
        int maxQueueSize = default(100);
        string rateControlModuleType = default("");

        string rxModule;
        string txModule;

        *.rateSelectionModule = "^.rateSelection";
        *.rxModule = "^." + rxModule;

        @class(Hcf);
        @display("i=block/queue;bgb=1311.142,783.852");

    submodules:

        edca: Edca {
            numEdcafs = numEdcafs;
            @display("p=83.83125,175.88126");
        }
        hcca: Hcca {
            @display("p=83.83125,335.325");
        }

        rateSelection: QoSRateSelection {
            parameters:
                @display("p=170.95001,498.05627");
        }
        rateControl: <rateControlModuleType> like IRateControl if rateControlModuleType != "" {
            parameters:
                @display("p=526,498.05627");
        }

        originatorMacDataService: OriginatorQoSMacDataService {
            @display("p=526,652.5688");
        }
        recipientMacDataService: RecipientQoSMacDataService {
            @display("p=170.95001,652.5688");
        }

        edcaMgmtAndNonQoSRecoveryProcedure: NonQoSRecoveryProcedure {
            cwCalculatorModule = "^.edca.edcaf[1]"; // best effort
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=616.40625,304.09375");
        }
        edcaDataRecoveryProcedures[numEdcafs]: QoSRecoveryProcedure {
            cwCalculatorModule = "^.edca.edcaf[" + string(index) + "]";
            rtsProcedureModule = "^.rtsProcedure";
            @display("p=616.40625,187.38751,row,50");
        }
        edcaTxopProcedures[numEdcafs]: TxopProcedure {
            @display("p=616.40625,78.9,row,50");
        }

        recipientAckPolicy: RecipientQoSAckPolicy {
            @display("p=1017.48126,187.38751");
        }
        originatorAckPolicy: OriginatorQoSAckPolicy {
            @display("p=1017.48126,78.9");
        }
        rtsPolicy: QoSRtsPolicy {
            @display("p=1017.48126,304.09375");
        }
        ctsPolicy: QoSCtsPolicy {
            @display("p=1017.48126,424.08752");
        }
        originatorBlockAckAgreementPolicy: OriginatorBlockAckAgreementPolicy {
            originatorAckPolicyModule = "^.originatorAckPolicy";
            @display("p=1017.48126,535.8625");
        }
        recipientBlockAckAgreementPolicy: RecipientBlockAckAgreementPolicy {
            @display("p=1017.48126,652.5688");
        }

}
