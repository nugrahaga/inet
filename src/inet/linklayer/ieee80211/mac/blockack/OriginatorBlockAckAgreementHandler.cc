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
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "OriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

void OriginatorBlockAckAgreementHandler::createAgreement(Ieee80211AddbaRequest *addbaRequest)
{
    OriginatorBlockAckAgreement *blockAckAgreement = new OriginatorBlockAckAgreement(addbaRequest->getReceiverAddress(), addbaRequest->getTid(), addbaRequest->getStartingSequenceNumber(), addbaRequest->getBufferSize(), addbaRequest->getAMsduSupported(), addbaRequest->getBlockAckPolicy() == 0);
    auto agreementId = std::make_pair(addbaRequest->getReceiverAddress(), addbaRequest->getTid());
    blockAckAgreements[agreementId] = blockAckAgreement;
}

Ieee80211AddbaRequest* OriginatorBlockAckAgreementHandler::buildAddbaRequest(MACAddress receiverAddr, Tid tid, int startingSequenceNumber, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy)
{
    Ieee80211AddbaRequest *addbaRequest = new Ieee80211AddbaRequest("AddbaReq");
    addbaRequest->setReceiverAddress(receiverAddr);
    addbaRequest->setTid(tid);
    addbaRequest->setAMsduSupported(blockAckAgreementPolicy->isMsduSupported());
    addbaRequest->setBlockAckTimeoutValue(blockAckAgreementPolicy->getBlockAckTimeoutValue());
    addbaRequest->setBufferSize(blockAckAgreementPolicy->getMaximumAllowedBufferSize());
    // The Block Ack Policy subfield is set to 1 for immediate Block Ack and 0 for delayed Block Ack.
    addbaRequest->setBlockAckPolicy(blockAckAgreementPolicy->isDelayedAckPolicySupported() ? 0 : 1);
    addbaRequest->setStartingSequenceNumber(startingSequenceNumber);
    return addbaRequest;
}

OriginatorBlockAckAgreement* OriginatorBlockAckAgreementHandler::getAgreement(MACAddress receiverAddr, Tid tid)
{
    auto agreementId = std::make_pair(receiverAddr, tid);
    auto it = blockAckAgreements.find(agreementId);
    return it != blockAckAgreements.end() ? it->second : nullptr;
}

Ieee80211Delba* OriginatorBlockAckAgreementHandler::buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode)
{
    Ieee80211Delba *delba = new Ieee80211Delba();
    delba->setReceiverAddress(receiverAddr);
    delba->setTid(tid);
    delba->setReasonCode(reasonCode);
    // The Initiator subfield indicates if the originator or the recipient of the data is sending this frame.
    delba->setInitiator(true);
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

void OriginatorBlockAckAgreementHandler::processTransmittedDataFrame(Ieee80211DataFrame* dataFrame, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy, IProcedureCallback* callback)
{
    auto agreement = getAgreement(dataFrame->getReceiverAddress(), dataFrame->getTid());
    if (blockAckAgreementPolicy->isAddbaReqNeeded(dataFrame) && agreement == nullptr) {
        auto addbaReq = buildAddbaRequest(dataFrame->getReceiverAddress(), dataFrame->getTid(), dataFrame->getSequenceNumber() + 1, blockAckAgreementPolicy);
        callback->processMgmtFrame(addbaReq);
    }
}

void OriginatorBlockAckAgreementHandler::processReceivedAddbaResp(Ieee80211AddbaResponse* addbaResp, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy, IProcedureCallback* callback)
{
    auto agreement = getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
    if (blockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, agreement)) {
        updateAgreement(agreement, addbaResp);
        blockAckAgreementPolicy->agreementEstablished(agreement);
    }
    else {
        // TODO: send a new one?
    }
}

void OriginatorBlockAckAgreementHandler::updateAgreement(OriginatorBlockAckAgreement* agreement, Ieee80211AddbaResponse* addbaResp)
{
    agreement->setIsAddbaResponseReceived(true);
    agreement->setBufferSize(addbaResp->getBufferSize());
    agreement->setBlockAckTimeoutValue(addbaResp->getBlockAckTimeoutValue());
}

void OriginatorBlockAckAgreementHandler::processTransmittedAddbaReq(Ieee80211AddbaRequest* addbaReq)
{
    createAgreement(addbaReq);
}

void OriginatorBlockAckAgreementHandler::processReceivedBlockAck(Ieee80211BlockAck* blockAck, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy)
{
    if (auto basicBlockAck = dynamic_cast<Ieee80211BasicBlockAck*>(blockAck)) {
        auto agreement = getAgreement(basicBlockAck->getTransmitterAddress(), basicBlockAck->getTidInfo());
        if (agreement)
            blockAckAgreementPolicy->blockAckReceived(agreement);
    }
    else
        throw cRuntimeError("Unsupported BlockAck");
}

void OriginatorBlockAckAgreementHandler::processTransmittedDelba(Ieee80211Delba* delba)
{
    terminateAgreement(delba->getReceiverAddress(), delba->getTid());
}

void OriginatorBlockAckAgreementHandler::processReceivedDelba(Ieee80211Delba* delba, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy)
{
    if (blockAckAgreementPolicy->isDelbaAccepted(delba))
        terminateAgreement(delba->getTransmitterAddress(), delba->getTid());
}

} // namespace ieee80211
}// namespace inet
