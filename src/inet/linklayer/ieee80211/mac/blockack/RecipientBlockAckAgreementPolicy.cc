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
#include "RecipientBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientBlockAckAgreementPolicy);

void RecipientBlockAckAgreementPolicy::initialize()
{
    isDelayedBlockAckPolicySupported = par("delayedAckPolicySupported");
    isAMsduSupported = par("aMsduSupported");
    maximumAllowedBufferSize = par("maximumAllowedBufferSize");
    blockAckTimeoutValue = par("blockAckTimeoutValue").doubleValue();

    agreementHandler = check_and_cast<RecipientBlockAckAgreementHandler*>(getModuleByPath(par("recipientBlockAckAgreementHandlerModule")));
}

//
// Every STA shall maintain an inactivity timer for every negotiated Block Ack setup. The inactivity timer at a
// recipient is reset when MPDUs corresponding to the TID for which the Block Ack policy is set are received
// and the Ack Policy subfield in the QoS Control field of that MPDU header is Block Ack or Implicit Block
// Ack Request.
//
void RecipientBlockAckAgreementPolicy::scheduleInactivityTimer(RecipientBlockAckAgreement *agreement)
{
    Enter_Method_Silent();
    simtime_t timeout = agreement->getBlockAckTimeoutValue();
    if (timeout != 0) {
        auto it = inacitivityTimers.find(std::make_pair(agreement->getBlockAckRecord()->getOriginatorAddress(), agreement->getBlockAckRecord()->getTid()));
        cMessage *timer = nullptr;
        if (it != inacitivityTimers.end())
            timer = it->second;
        else {
            timer = new cMessage("InactivityTimer");
            inacitivityTimers[std::make_pair(agreement->getBlockAckRecord()->getOriginatorAddress(), agreement->getBlockAckRecord()->getTid())] = timer;
        }
        cancelEvent(timer);
        scheduleAt(simTime() + timeout, timer);
    }
}

bool RecipientBlockAckAgreementPolicy::isAddbaReqAccepted(Ieee80211AddbaRequest* addbaReq)
{
    return true;
}

void RecipientBlockAckAgreementPolicy::handleMessage(cMessage* msg)
{
    // The Block Ack Timeout Value field contains the duration, in TUs, after which the Block Ack setup is
    // terminated, if there are no frame exchanges (see 10.5.4) within this duration using this Block Ack
    // agreement. A value of 0 disables the timeout.
    auto id = findAgreement(msg);
    auto hcf = check_and_cast<Hcf*>(getParentModule()); // FIXME: khm
    Tid tid = id.second;
    MACAddress originatorAddr = id.first;
    hcf->processUpperFrame(agreementHandler->buildDelba(originatorAddr, tid, 39)); // 39 - TIMEOUT see: Table 8-36—Reason codes
}

void RecipientBlockAckAgreementPolicy::agreementEstablished(RecipientBlockAckAgreement* agreement)
{
    scheduleInactivityTimer(agreement);
}

//
// The inactivity timer at a recipient is reset when MPDUs corresponding to the TID for which the Block Ack
// policy is set are received and the Ack Policy subfield in the QoS Control field of that MPDU header is
// Block Ack or Implicit Block Ack Request.
//
void RecipientBlockAckAgreementPolicy::qosFrameReceived(Ieee80211DataFrame* qosFrame)
{
    if (qosFrame->getAckPolicy() == AckPolicy::BLOCK_ACK) { // TODO: + Implicit Block Ack
        Tid tid = qosFrame->getTid();
        MACAddress originatorAddr = qosFrame->getTransmitterAddress();
        auto agreement = agreementHandler->getAgreement(tid, originatorAddr);
        if (agreement)
            scheduleInactivityTimer(agreement);
    }
}

bool RecipientBlockAckAgreementPolicy::isDelbaAccepted(Ieee80211Delba* delba)
{
    return true;
}

std::pair<MACAddress, Tid> RecipientBlockAckAgreementPolicy::findAgreement(cMessage* inactivityTimer)
{
    for (auto timer : inacitivityTimers) {
        if (timer.second == inactivityTimer)
            return timer.first;
    }
    throw cRuntimeError("Agreement not found");
}

RecipientBlockAckAgreementPolicy::~RecipientBlockAckAgreementPolicy()
{
    for (auto timer : inacitivityTimers) {
        cancelAndDelete(timer.second);
    }
}

} /* namespace ieee80211 */
} /* namespace inet */
