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

#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "RecipientBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientBlockAckAgreementHandler);

void RecipientBlockAckAgreementHandler::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        rateSelection = dynamic_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        sifs = rateSelection->getSlowestMandatoryMode()->getSifsTime();
    }
}

//
// An originator that intends to use the Block Ack mechanism for the transmission of QoS data frames to an
// intended recipient should first check whether the intended recipient STA is capable of participating in Block
// Ack mechanism by discovering and examining its Delayed Block Ack and Immediate Block Ack capability
// bits. If the intended recipient STA is capable of participating, the originator sends an ADDBA Request frame
// indicating the TID for which the Block Ack is being set up.
//
RecipientBlockAckAgreement* RecipientBlockAckAgreementHandler::addAgreement(Ieee80211AddbaRequest* addbaReq)
{
    MACAddress originatorAddr = addbaReq->getTransmitterAddress();
    auto id = std::make_pair(originatorAddr, addbaReq->getTid());
    auto it = blockAckAgreements.find(id);
    if (it == blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = new RecipientBlockAckAgreement(originatorAddr, addbaReq->getTid(), addbaReq->getStartingSequenceNumber(), addbaReq->getBufferSize(), addbaReq->getBlockAckTimeoutValue());
        blockAckAgreements[id] = agreement;
        return agreement;
    }
    else
        throw cRuntimeError("TODO"); // TODO: update?
}

//
// When a timeout of BlockAckTimeout is detected, the STA shall send a DELBA frame to the peer STA with the Reason Code
// field set to TIMEOUT and shall issue a MLME-DELBA.indication primitive with the ReasonCode
// parameter having a value of TIMEOUT. The procedure is illustrated in Figure 10-14.
//
Ieee80211Delba* RecipientBlockAckAgreementHandler::buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode)
{
    Ieee80211Delba *delba = new Ieee80211Delba("Delba");
    delba->setReceiverAddress(receiverAddr);
    delba->setInitiator(false);
    delba->setTid(tid);
    delba->setReasonCode(reasonCode);
    delba->setDuration(rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK) + sifs);
    return delba;
}

Ieee80211AddbaResponse* RecipientBlockAckAgreementHandler::buildAddbaResponse(Ieee80211AddbaRequest* frame, bool aMsduSupported, simtime_t blockAckTimeoutValue, int maximumAllowedBufferSize, bool delayedBlockAckPolicySupported)
{
    Ieee80211AddbaResponse *addbaResponse = new Ieee80211AddbaResponse("AddbaResponse");
    addbaResponse->setReceiverAddress(frame->getTransmitterAddress());
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    Tid tid = frame->getTid();
    addbaResponse->setTid(tid);
    addbaResponse->setBlockAckPolicy(!frame->getBlockAckPolicy() && delayedBlockAckPolicySupported ? false : true);
    addbaResponse->setBufferSize(frame->getBufferSize() <= maximumAllowedBufferSize ? frame->getBufferSize() : maximumAllowedBufferSize);
    addbaResponse->setBlockAckTimeoutValue(blockAckTimeoutValue == 0 ? blockAckTimeoutValue : frame->getBlockAckTimeoutValue());
    addbaResponse->setAMsduSupported(aMsduSupported);
    addbaResponse->setDuration(rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK) + sifs);
    return addbaResponse;
}

void RecipientBlockAckAgreementHandler::updateAgreement(Ieee80211AddbaResponse *frame)
{
    auto id = std::make_pair(frame->getReceiverAddress(), frame->getTid());
    auto it = blockAckAgreements.find(id);
    if (it != blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = it->second;
        agreement->addbaResposneSent();
    }
    else
        throw cRuntimeError("Agreement is not found");
}

void RecipientBlockAckAgreementHandler::terminateAgreement(MACAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        RecipientBlockAckAgreement *agreement = it->second;
        blockAckAgreements.erase(it);
        delete agreement;
    }
}

RecipientBlockAckAgreement* RecipientBlockAckAgreementHandler::getAgreement(Tid tid, MACAddress originatorAddr)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}


} // namespace ieee80211
} // namespace inet
