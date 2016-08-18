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

#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "OriginatorBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorBlockAckAgreementPolicy);

void OriginatorBlockAckAgreementPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ackPolicy = check_and_cast<OriginatorQoSAckPolicy*>(getModuleByPath(par("originatorQoSAckPolicyModule")));
        delayedAckPolicySupported = par("delayedAckPolicySupported");
        aMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue").doubleValue();
        addbaFailureTimeout = par("addbaFailureTimeout");
    }
}

void OriginatorBlockAckAgreementPolicy::handleMessage(cMessage* msg)
{
    // When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the
    // peer STA with the Reason Code field set to TIMEOUT and shall issue a MLME-DELBA.indication
    // primitive with the ReasonCode parameter having a value of TIMEOUT.
    // The procedure is illustrated in Figure 10-14.
    auto id = findAgreement(msg);
    auto hcf = check_and_cast<Hcf*>(getParentModule()); // FIXME: khm
    Tid tid = id.second;
    MACAddress receiverAddr = id.first;
    hcf->processUpperFrame(agreementHandler->buildDelba(receiverAddr, tid, 39)); // 39 - TIMEOUT see: Table 8-36—Reason codes
}

simtime_t OriginatorBlockAckAgreementPolicy::getAddbaFailureTimeout() const
{
    // TODO: ADDBAFailureTimeout -- 6.3.29.2.2 Semantics of the service primitive
    throw cRuntimeError("Unimplemented");
}

void OriginatorBlockAckAgreementPolicy::scheduleInactivityTimer(OriginatorBlockAckAgreement* agreement)
{
    Enter_Method_Silent();
    simtime_t timeout = agreement->getBlockAckTimeoutValue();
    if (timeout != 0) {
        auto it = inacitivityTimers.find(std::make_pair(agreement->getReceiverAddr(), agreement->getTid()));
        cMessage *timer = nullptr;
        if (it != inacitivityTimers.end())
            timer = it->second;
        else {
            timer = new cMessage("InactivityTimer");
            inacitivityTimers[std::make_pair(agreement->getReceiverAddr(), agreement->getTid())] = timer;
        }
        cancelEvent(timer);
        scheduleAt(simTime() + timeout, timer);
    }
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqNeeded(Ieee80211DataFrame* frame)
{
    return ackPolicy->isBlockAckPolicyEligibleFrame(frame);
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqAccepted(Ieee80211AddbaResponse* addbaResp, OriginatorBlockAckAgreement* agreement)
{
    ASSERT(agreement);
    return true;
}

bool OriginatorBlockAckAgreementPolicy::isDelbaAccepted(Ieee80211Delba* delba)
{
    return true;
}

//
// The inactivity timer at the originator is reset when a BlockAck frame
// corresponding to the TID for which the Block Ack policy is set is received.
//
void OriginatorBlockAckAgreementPolicy::blockAckReceived(OriginatorBlockAckAgreement* agreement)
{
    scheduleInactivityTimer(agreement);
}

std::pair<MACAddress, Tid> OriginatorBlockAckAgreementPolicy::findAgreement(cMessage* inactivityTimer)
{
    for (auto timer : inacitivityTimers) {
        if (timer.second == inactivityTimer)
            return timer.first;
    }
    throw cRuntimeError("Agreement not found");
}

OriginatorBlockAckAgreementPolicy::~OriginatorBlockAckAgreementPolicy()
{
    for (auto timer : inacitivityTimers) {
        cancelAndDelete(timer.second);
    }
}

void OriginatorBlockAckAgreementPolicy::agreementEstablished(OriginatorBlockAckAgreement* agreement)
{
    scheduleInactivityTimer(agreement);
}

} /* namespace ieee80211 */
} /* namespace inet */
