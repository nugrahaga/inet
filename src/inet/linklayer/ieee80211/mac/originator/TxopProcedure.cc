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
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "TxopProcedure.h"

namespace inet {
namespace ieee80211 {

Define_Module(TxopProcedure);

void TxopProcedure::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(NF_MODESET_CHANGED, this);
        limit = par("txopLimit");
    }
}

s TxopProcedure::getTxopLimit(const IIeee80211Mode *mode, AccessCategory ac)
{
    switch (ac)
    {
        case AC_BK: return s(0);
        case AC_BE: return s(0);
        case AC_VI:
            if (dynamic_cast<const Ieee80211DsssMode*>(mode) || dynamic_cast<const Ieee80211HrDsssMode*>(mode)) return ms(6.016);
            else if (dynamic_cast<const Ieee80211HTMode*>(mode) || dynamic_cast<const Ieee80211OFDMMode*>(mode)) return ms(3.008);
            else return s(0);
        case AC_VO:
            if (dynamic_cast<const Ieee80211DsssMode*>(mode) || dynamic_cast<const Ieee80211HrDsssMode*>(mode)) return ms(3.264);
            else if (dynamic_cast<const Ieee80211HTMode*>(mode) || dynamic_cast<const Ieee80211OFDMMode*>(mode)) return ms(1.504);
            else return s(0);
        default: throw cRuntimeError("Unknown access category = %d", ac);
    }
}

simtime_t TxopProcedure::getStart() const
{
    return start;
}

simtime_t TxopProcedure::getLimit() const
{
    return limit;
}

void TxopProcedure::startTxop(AccessCategory ac)
{
    if (start != -1)
        throw cRuntimeError("Txop is already running");
    if (limit == -1) {
        auto referenceMode = modeSet->getSlowestMandatoryMode();
        limit = getTxopLimit(referenceMode, ac).get();
    }
    start = simTime();
}


void TxopProcedure::stopTxop()
{
    start = -1;
}

simtime_t TxopProcedure::getRemaining() const
{
    if (start == -1)
        throw cRuntimeError("Txop has not started yet");
    auto now = simTime();
    return now > start + limit ? 0 : now - start;
}

// FIXME: implement!
bool TxopProcedure::isFinalFragment(Ieee80211Frame* frame)
{
    return false;
}

// FIXME: implement!
bool TxopProcedure::isTxopInitiator(Ieee80211Frame* frame)
{
    return false;
}

// FIXME: implement!
bool TxopProcedure::isTxopTerminator(Ieee80211Frame* frame)
{
    return false;
}

void TxopProcedure::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

} // namespace ieee80211
} // namespace inet

