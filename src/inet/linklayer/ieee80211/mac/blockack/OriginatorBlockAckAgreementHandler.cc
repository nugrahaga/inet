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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "OriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorBlockAckAgreementHandler);

void OriginatorBlockAckAgreementHandler::initialize(int stage)
{
    if (stage == INITSTAGE_LAST) {
        rateSelection = dynamic_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        sifs = rateSelection->getSlowestMandatoryMode()->getSifsTime();
        slotTime = rateSelection->getSlowestMandatoryMode()->getSlotTime();
        phyRxStartDelay = rateSelection->getSlowestMandatoryMode()->getPhyRxStartDelay();
    }
}

void OriginatorBlockAckAgreementHandler::createAgreement(Ieee80211AddbaRequest *addbaRequest)
{
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreements[agreementId] = blockAckAgreement;
}

simtime_t OriginatorBlockAckAgreementHandler::getAddbaRequestDuration(Ieee80211AddbaRequest *addbaReq) const
{
    const IIeee80211Mode *mode = rateSelection->getModeForUnicastDataOrMgmtFrame(addbaReq);
    return mode->getDuration(addbaReq->getBitLength());
}

simtime_t OriginatorBlockAckAgreementHandler::getAddbaRequestEarlyTimeout() const
{
    return sifs + slotTime + phyRxStartDelay;
}

Ieee80211AddbaRequest* OriginatorBlockAckAgreementHandler::buildAddbaRequest(MACAddress receiverAddr, Tid tid, int startingSequenceNumber, bool aMsduSupported, simtime_t blockAckTimeoutValue, int maximumAllowedBufferSize, bool delayedBlockAckPolicySupported)
{
    Ieee80211AddbaRequest *addbaRequest = new Ieee80211AddbaRequest("AddbaReq");
    addbaRequest->setReceiverAddress(receiverAddr);
    addbaRequest->setTid(tid);
    addbaRequest->setAMsduSupported(aMsduSupported);
    addbaRequest->setBlockAckTimeoutValue(blockAckTimeoutValue);
    addbaRequest->setBufferSize(maximumAllowedBufferSize);
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    addbaRequest->setBlockAckPolicy(delayedBlockAckPolicySupported ? 0 : 1);
    addbaRequest->setStartingSequenceNumber(startingSequenceNumber);
    setFrameMode(addbaRequest, rateSelection->getModeForUnicastDataOrMgmtFrame(addbaRequest));
    // Within all management frames sent by the QoS STA, the Duration field contains a duration
    // value as defined in 8.2.5.
    addbaRequest->setDuration(rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK) + sifs);
    return addbaRequest;
}

OriginatorBlockAckAgreement* OriginatorBlockAckAgreementHandler::getAgreement(MACAddress receiverAddr, Tid tid)
{
    auto agreementId = std::make_pair(receiverAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}

OriginatorBlockAckAgreement* OriginatorBlockAckAgreementHandler::getAgreement(cMessage* inactivityTimer)
{
    for (auto it : blockAckAgreements) {
        auto *agreement = it.second;
        if (agreement->getInactivityTimer() == inactivityTimer)
            return agreement;
    }
    throw cRuntimeError("Agreement not found");
}

Ieee80211Delba* OriginatorBlockAckAgreementHandler::buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode)
{
    Ieee80211Delba *delba = new Ieee80211Delba();
    delba->setReceiverAddress(receiverAddr);
    delba->setTid(tid);
    delba->setReasonCode(reasonCode);
    // The Initiator subfield indicates if the originator or the recipient of the data is sending this frame.
    delba->setInitiator(true);
    delba->setDuration(rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK) + sifs);
    return delba;
}

void OriginatorBlockAckAgreementHandler::terminateAgreement(MACAddress originatorAddr, Tid tid)
{
    auto agreementId = std::make_pair(originatorAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    if (it != blockAckAgreements.end()) {
        OriginatorBlockAckAgreement *agreement = it->second;
        blockAckAgreements.erase(it);
        delete agreement;
    }
}

void OriginatorBlockAckAgreementHandler::updateAgreement(OriginatorBlockAckAgreement* agreement, Ieee80211AddbaResponse* addbaResp)
{
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
}

// TODO: move this part to somewhere else
Ieee80211Frame *OriginatorBlockAckAgreementHandler::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
    return frame;
}

} // namespace ieee80211
} // namespace inet
