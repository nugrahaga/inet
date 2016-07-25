//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

namespace inet {
namespace ieee80211 {

Define_Module(Hcf);

void Hcf::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        numEdcafs = par("numEdcafs");
        edca = check_and_cast<Edca *>(getSubmodule("edca"));
        hcca = check_and_cast<Hcca *>(getSubmodule("hcca"));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        originatorBlockAckAgreementHandler = check_and_cast<OriginatorBlockAckAgreementHandler *>(getSubmodule("originatorBlockAckAgreementHandler"));
        recipientBlockAckAgreementHandler = check_and_cast<RecipientBlockAckAgreementHandler*>(getSubmodule("recipientBlockAckAgreementHandler"));
        rtsProcedure = check_and_cast<RtsProcedure*>(getSubmodule("rtsProcedure"));
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this));
        rateSelection = check_and_cast<IRateSelection *>(getModuleByPath(par("rateSelectionModule")));
        referenceMode = rateSelection->getSlowestMandatoryMode();
        frameSequenceHandler = check_and_cast<FrameSequenceHandler *>(getSubmodule("frameSequenceHandler"));
        sifs = referenceMode->getSifsTime();
        originatorDataService = check_and_cast<OriginatorQoSMacDataService *>(getSubmodule(("originatorQoSMacDataService")));
        recipientDataService = check_and_cast<RecipientQoSMacDataService*>(getSubmodule("recipientQoSMacDataService"));
        originatorAckProcedure = new OriginatorAckProcedure(rateSelection);
        recipientAckProcedure = new RecipientAckProcedure(rateSelection);
        ctsProcedure = new CtsProcedure(rx, rateSelection);
        originatorBlockAckProcedure = new OriginatorBlockAckProcedure(rateSelection);
        recipientBlockAckProcedure = new RecipientBlockAckProcedure(recipientBlockAckAgreementHandler, rateSelection);
        lifetimeHandler = new EdcaTransmitLifetimeHandler(0, 0, 0, 0); // FIXME: needs only one timeout parameter
        for (int ac = 0; ac < numEdcafs; ac++) {
            edcaPendingQueues.push_back(new PendingQueue(par("maxQueueSize"), nullptr));
            edcaRecoveryProcedures.push_back(check_and_cast<RecoveryProcedure *>(getSubmodule("edcaRecoveryProcedures", ac)));
            edcaAckHandlers.push_back(new AckHandler());
            edcaInProgressFrames.push_back(new InProgressFrames(edcaPendingQueues[ac], originatorDataService, edcaAckHandlers[ac]));
            edcaTxops.push_back(check_and_cast<TxopProcedure *>(getSubmodule("edcaTxopProcedures", ac)));
        }
    }
}

void Hcf::processUpperFrame(Ieee80211DataOrMgmtFrame* frame)
{
    AccessCategory ac = AccessCategory(-1);
    if (dynamic_cast<Ieee80211ManagementFrame *>(frame)) // TODO: + non-QoS frames
        ac = AccessCategory::AC_BE;
    else if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(frame))
        ac = edca->classifyFrame(dataFrame);
    else
        throw cRuntimeError("Unknown message type");
    if (edcaPendingQueues[ac]->insert(frame)) {
        EV_INFO << "Frame " << frame->getName() << " has been inserted into the PendingQueue." << endl;
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac)
            edca->requestChannelAccess(ac, this);
    }
    else {
        EV_INFO << "Frame " << frame->getName() << " has been dropped because the PendingQueue is full." << endl;
        delete frame;
    }
}

void Hcf::processLowerFrame(Ieee80211Frame* frame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf && frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->processResponse(frame);
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        recipientProcessReceivedFrame(frame);
}

void Hcf::channelGranted(IChannelAccess* channelAccess)
{
    auto edcaf = check_and_cast<Edcaf*>(channelAccess);
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        edcaTxops[ac]->startTxop(ac);
        startFrameSequence(ac);
        handleInternalCollision(edca->getInternallyCollidedEdcafs());
    }
    else
        throw cRuntimeError("Channel access granted but channel owner not found!");
}

FrameSequenceContext* Hcf::buildContext(AccessCategory ac)
{
    return new FrameSequenceContext(edcaPendingQueues[AccessCategory::AC_BE], edcaInProgressFrames[ac], originatorAckProcedure, rtsProcedure, edcaTxops[ac], originatorBlockAckProcedure, originatorBlockAckAgreementHandler, rateSelection->getSlowestMandatoryMode());
}

void Hcf::startFrameSequence(AccessCategory ac)
{
    frameSequenceHandler->startFrameSequence(new HcfFs(), buildContext(ac), this);
}

void Hcf::handleInternalCollision(std::vector<Edcaf*> internallyCollidedEdcafs)
{
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        Ieee80211DataOrMgmtFrame *internallyCollidedFrame = edcaInProgressFrames[ac]->getFrameToTransmit();
        edcaRecoveryProcedures[ac]->dataOrMgmtFrameTransmissionFailed(internallyCollidedFrame);
        if (edcaRecoveryProcedures[ac]->isDataOrMgtmFrameRetryLimitReached(internallyCollidedFrame))
            edcaInProgressFrames[ac]->dropFrame(internallyCollidedFrame);
        else
            edcaf->requestChannel(this);
    }
}

void Hcf::frameSequenceFinished()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        bool startContention = hasFrameToTransmit();
        edcaf->releaseChannel(this);
        AccessCategory ac = edcaf->getAccessCategory();
        edcaTxops[ac]->stopTxop();
        if (startContention)
            edcaf->requestChannel(this);
    }
    else if (hcca->isOwning()) {
        hcca->releaseChannel(this);
        throw cRuntimeError("Hcca is unimplemented!");
    }
    else
        throw cRuntimeError("Frame sequence finished but channel owner not found!");
}

void Hcf::recipientProcessReceivedFrame(Ieee80211Frame* frame)
{
    recipientAckProcedure->processReceivedFrame(frame);
    auto ack = recipientAckProcedure->buildAck(frame);
    if (ack) {
        tx->transmitFrame(ack, sifs, this);
        recipientAckProcedure->processTransmittedAck(ack);
    }
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame)) {
        sendUp(recipientDataService->dataFrameReceived(dataFrame));
    }
    else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(frame)) {
        sendUp(recipientDataService->managementFrameReceived(mgmtFrame));
        recipientProcessReceivedManagementFrame(mgmtFrame);
    }
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(frame));
        recipientProcessReceivedControlFrame(frame);
    }
}

void Hcf::recipientProcessReceivedControlFrame(Ieee80211Frame* frame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame)) {
        ctsProcedure->processReceivedRts(rtsFrame);
        auto ctsFrame = ctsProcedure->buildCts(rtsFrame);
        if (ctsFrame) {
            tx->transmitFrame(ctsFrame, sifs, this);
            ctsProcedure->processTransmittedCts(ctsFrame);
        }
    }
    else if (auto blockAckRequest = dynamic_cast<Ieee80211BlockAckReq*>(frame)) {
        recipientBlockAckProcedure->processReceivedBlockAckReq(blockAckRequest);
        auto blockAck = recipientBlockAckProcedure->buildBlockAck(blockAckRequest);
        if (blockAck) {
            tx->transmitFrame(blockAck, sifs, this);
            recipientBlockAckProcedure->processTransmittedBlockAck(blockAck);
        }
    }
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::recipientProcessReceivedManagementFrame(Ieee80211ManagementFrame* frame)
{
    if (auto addbaRequest = dynamic_cast<Ieee80211AddbaRequest *>(frame)) {
        recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest);
        auto addbaResponse = recipientBlockAckAgreementHandler->buildAddbaResponse(addbaRequest);
        if (addbaResponse)
            processUpperFrame(addbaResponse);
    }
    else if (auto addbaResp = dynamic_cast<Ieee80211AddbaResponse *>(frame))
        originatorBlockAckAgreementHandler->processReceivedAddbaResponse(addbaResp);
    else if (auto delba = dynamic_cast<Ieee80211Delba*>(frame)) {
        if (delba->getInitiator())
            recipientBlockAckAgreementHandler->processReceivedDelba(delba);
        else
            originatorBlockAckAgreementHandler->processReceivedDelba(delba);
    }
    else
        throw cRuntimeError("Unknown management frame");
}

void Hcf::transmissionComplete()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        frameSequenceHandler->transmissionComplete();
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else
        ;
}

void Hcf::originatorProcessRtsProtectionFailed(Ieee80211DataOrMgmtFrame* protectedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        AccessCategory ac = edcaf->getAccessCategory();
        edcaRecoveryProcedures[ac]->rtsFrameTransmissionFailed(protectedFrame);
        bool retryLimitReached = edcaRecoveryProcedures[ac]->isRtsFrameRetryLimitReached(protectedFrame);
        if (retryLimitReached) {
            edcaInProgressFrames[ac]->dropFrame(protectedFrame);
            delete protectedFrame;
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Ieee80211Frame* transmittedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (transmittedFrame->getReceiverAddress().isMulticast())
            edcaRecoveryProcedures[ac]->multicastFrameTransmitted();
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(transmittedFrame))
            originatorProcessTransmittedDataFrame(dataFrame, ac);
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame*>(transmittedFrame))
            originatorProcessTransmittedManagementFrame(mgmtFrame, ac);
        else // TODO: Ieee80211ControlFrame
            originatorProcessTransmittedControlFrame(transmittedFrame, ac);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented");
    else
        throw cRuntimeError("Frame transmitted but channel owner not found");
}

void Hcf::originatorProcessTransmittedDataFrame(Ieee80211DataFrame* dataFrame, AccessCategory ac)
{
    edcaAckHandlers[ac]->processTransmittedQoSData(dataFrame);
    if (dataFrame->getAckPolicy() == NO_ACK)
        edcaInProgressFrames[ac]->dropFrame(dataFrame);
}

void Hcf::originatorProcessTransmittedManagementFrame(Ieee80211ManagementFrame* mgmtFrame, AccessCategory ac)
{
    if (auto addbaReq = dynamic_cast<Ieee80211AddbaRequest*>(mgmtFrame)) {
        edcaAckHandlers[ac]->processTransmittedMgmtFrame(addbaReq);
        // TODO: frame sequence => originatorBlockAckAgreementHandler->processAddbaRequest(addbaReq);
    }
    else if (auto addbaResp = dynamic_cast<Ieee80211AddbaResponse*>(mgmtFrame)) {
        edcaAckHandlers[ac]->processTransmittedMgmtFrame(addbaResp);
        recipientBlockAckAgreementHandler->processTransmittedAddbaResponse(addbaResp);
    }
    else if (auto delba = dynamic_cast<Ieee80211Delba *>(mgmtFrame)) {
        edcaAckHandlers[ac]->processTransmittedMgmtFrame(delba);
    }
    else
        throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessTransmittedControlFrame(Ieee80211Frame* controlFrame, AccessCategory ac)
{
    if (auto blockAckReq = dynamic_cast<Ieee80211BlockAckReq*>(controlFrame))
        edcaAckHandlers[ac]->processTransmittedBlockAckReq(blockAckReq);
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessFailedFrame(Ieee80211DataOrMgmtFrame* failedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(failedFrame)) {
            ASSERT(dataFrame->getType() == ST_DATA_WITH_QOS);
            EV_INFO << "QoS Data frame transmission failed\n";
            if (dataFrame->getAckPolicy() == NORMAL_ACK) {
                edcaRecoveryProcedures[ac]->dataOrMgmtFrameTransmissionFailed(dataFrame);
                bool retryLimitReached = edcaRecoveryProcedures[ac]->isDataOrMgtmFrameRetryLimitReached(dataFrame);
                if (retryLimitReached) {
                    edcaInProgressFrames[ac]->dropFrame(dataFrame);
                    delete dataFrame;
                }
            }
            else if (dataFrame->getAckPolicy() == BLOCK_ACK) {
                // TODO:
                // bool lifetimeExpired = lifetimeHandler->isLifetimeExpired(failedFrame);
                // if (lifetimeExpired) {
                //    inProgressFrames->dropFrame(failedFrame);
                //    delete dataFrame;
                // }
            }
            else
                throw cRuntimeError("Unimplemented!");
        }
        else {
            throw cRuntimeError("Unimplemented"); // failed block ack req, addba req
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame)
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
            originatorProcessReceivedDataFrame(dataFrame, lastTransmittedFrame, ac);
        else if (auto mgmtFrame = dynamic_cast<Ieee80211ManagementFrame *>(frame))
            originatorProcessReceivedManagementFrame(mgmtFrame, lastTransmittedFrame, ac);
        else
            originatorProcessReceivedControlFrame(frame, lastTransmittedFrame, ac);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedManagementFrame(Ieee80211ManagementFrame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
{
    throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessReceivedControlFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
{
    if (auto ackFrame = dynamic_cast<Ieee80211ACKFrame *>(frame)) {
        edcaRecoveryProcedures[ac]->ackFrameReceived(check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
        edcaAckHandlers[ac]->processReceivedAck(ackFrame, check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
        edcaInProgressFrames[ac]->dropFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
    }
    else if (auto blockAck = dynamic_cast<Ieee80211BlockAck *>(frame)) {
        EV_INFO << "BlockAck has arrived" << std::endl;
        edcaRecoveryProcedures[ac]->blockAckFrameReceived();
        auto ackedSeqAndFragNums = edcaAckHandlers[ac]->processReceivedBlockAck(blockAck);
        originatorBlockAckAgreementHandler->processReceivedBlockAck(blockAck);
        EV_INFO << "It has acknowledged the following frames:" << std::endl;
        for (auto seqCtrlField : ackedSeqAndFragNums)
            EV_INFO << "Fragment number = " << seqCtrlField.getSequenceNumber() << " Sequence number = " << (int)seqCtrlField.getFragmentNumber() << std::endl;
        edcaInProgressFrames[ac]->dropFrames(ackedSeqAndFragNums);
    }
    else if (auto addbaResponse = dynamic_cast<Ieee80211AddbaResponse*>(frame)) {
        recipientBlockAckAgreementHandler->processTransmittedAddbaResponse(addbaResponse);
    }
    else if (dynamic_cast<Ieee80211RTSFrame*>(frame))
        ; // void
    else if (dynamic_cast<Ieee80211CTSFrame*>(frame))
        edcaRecoveryProcedures[ac]->ctsFrameReceived();
    else if (frame->getType() == ST_DATA_WITH_QOS)
        ; // void
    else if (dynamic_cast<Ieee80211BasicBlockAckReq*>(frame))
        ; // void
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessReceivedDataFrame(Ieee80211DataFrame* frame, Ieee80211Frame* lastTransmittedFrame, AccessCategory ac)
{
    throw cRuntimeError("Unknown data frame");
}

bool Hcf::hasFrameToTransmit()
{
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        return !edcaPendingQueues[ac]->isEmpty() || edcaInProgressFrames[ac]->hasInProgressFrames();
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::sendUp(const std::vector<Ieee80211Frame*>& completeFrames)
{
    for (auto frame : completeFrames) {
        // FIXME: mgmt module does not handle addba req ..
        if (!dynamic_cast<Ieee80211AddbaRequest*>(frame) && !dynamic_cast<Ieee80211AddbaResponse*>(frame))
            mac->sendUp(frame);
    }
}

bool Hcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

void Hcf::transmitFrame(Ieee80211Frame* frame, simtime_t ifs)
{
    tx->transmitFrame(frame, ifs, this);
}

Hcf::~Hcf()
{
    delete originatorAckProcedure;
    delete recipientAckProcedure;
    delete ctsProcedure;
    for (auto inProgressFrames : edcaInProgressFrames)
        delete inProgressFrames;
    for (auto pendingQueue : edcaPendingQueues)
        delete pendingQueue;
    for (auto ackHandler : edcaAckHandlers)
        delete ackHandler;
}

} // namespace ieee80211
} // namespace inet
