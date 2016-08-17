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
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.h"
#include "inet/linklayer/ieee80211/mac/framesequence/DcfFs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

namespace inet {
namespace ieee80211 {

Define_Module(Dcf);

void Dcf::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this));
        originatorDataService = check_and_cast<OriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<RecipientMacDataService*>(getSubmodule("recipientMacDataService"));
        frameSequenceHandler = check_and_cast<FrameSequenceHandler *>(getSubmodule("frameSequenceHandler"));
        recoveryProcedure = check_and_cast<NonQoSRecoveryProcedure *>(getSubmodule("recoveryProcedure"));
        rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
        rtsProcedure = check_and_cast<RtsProcedure*>(getSubmodule("rtsProcedure"));
        dcfChannelAccess = check_and_cast<Dcaf *>(getSubmodule("channelAccess"));
        // TODO: sifs
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        pendingQueue = new PendingQueue(par("maxQueueSize"), nullptr);
        ackHandler = new AckHandler();
        //recipientAckProcedure = new RecipientAckProcedure(rateSelection);
        inProgressFrames = new InProgressFrames(pendingQueue, originatorDataService, ackHandler);
        //ctsProcedure = new CtsProcedure(rx, rateSelection);
        stationRetryCounters = new StationRetryCounters();
    }
}

void Dcf::channelGranted(IChannelAccess *channelAccess)
{
    ASSERT(dcfChannelAccess == channelAccess);
    if (!frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->startFrameSequence(new DcfFs(), buildContext(), this);
}

void Dcf::processUpperFrame(Ieee80211DataOrMgmtFrame* frame)
{
    if (pendingQueue->insert(frame)) {
        EV_INFO << "Frame " << frame->getName() << " has been inserted into the PendingQueue." << endl;
        dcfChannelAccess->requestChannel(this);
    }
    else {
        EV_INFO << "Frame " << frame->getName() << " has been dropped because the PendingQueue is full." << endl;
        delete frame;
    }
}

void Dcf::transmitControlResponseFrame(Ieee80211Frame* responseFrame, Ieee80211Frame* receivedFrame)
{
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseCtsFrameMode(rtsFrame);
    else if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(receivedFrame))
        responseMode = rateSelection->computeResponseAckFrameMode(dataOrMgmtFrame);
    else
        throw cRuntimeError("Unknown received frame type");
    setFrameMode(responseFrame, responseMode);
    tx->transmitFrame(responseFrame, -1, this); // FIXME
}

void Dcf::processMgmtFrame(Ieee80211ManagementFrame* mgmtFrame)
{
    throw cRuntimeError("Unknown management frame");
}

void Dcf::recipientProcessTransmittedControlResponseFrame(Ieee80211Frame* frame)
{
    if (auto ctsFrame = dynamic_cast<Ieee80211CTSFrame*>(frame))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto ackFrame = dynamic_cast<Ieee80211ACKFrame*>(frame))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Dcf::processLowerFrame(Ieee80211Frame* frame)
{
    if (frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->processResponse(frame);
    else
        recipientProcessReceivedFrame(frame);
}

void Dcf::transmitFrame(Ieee80211Frame* frame, simtime_t ifs)
{
    setFrameMode(frame, rateSelection->computeMode(frame));
    tx->transmitFrame(frame, ifs, this);
}

void Dcf::frameSequenceFinished()
{
    dcfChannelAccess->releaseChannel(this);
    if (hasFrameToTransmit())
        dcfChannelAccess->requestChannel(this);
}

bool Dcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

void Dcf::recipientProcessReceivedFrame(Ieee80211Frame* frame)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        recipientAckProcedure->processReceivedFrame(dataOrMgmtFrame, recipientAckPolicy, this);
    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame))
        sendUp(recipientDataService->dataFrameReceived(dataFrame));
    else { // TODO: else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(frame));
        recipientProcessControlFrame(frame);
    }
}

void Dcf::sendUp(const std::vector<Ieee80211Frame*>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUp(frame);
}

void Dcf::recipientProcessControlFrame(Ieee80211Frame* frame)
{
    if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        ctsProcedure->processReceivedRts(rtsFrame, ctsPolicy, this);
    else
        throw cRuntimeError("Unknown control frame");
}

FrameSequenceContext* Dcf::buildContext()
{
    return new FrameSequenceContext(modeSet, inProgressFrames, nullptr, nullptr);
}

void Dcf::transmissionComplete(Ieee80211Frame *frame)
{
    if (frameSequenceHandler->isSequenceRunning())
        frameSequenceHandler->transmissionComplete();
    else ;
}

bool Dcf::hasFrameToTransmit()
{
    return !pendingQueue->isEmpty() || inProgressFrames->hasInProgressFrames();
}

void Dcf::originatorProcessRtsProtectionFailed(Ieee80211DataOrMgmtFrame* protectedFrame)
{
    EV_INFO << "RTS frame transmission failed\n";
    recoveryProcedure->rtsFrameTransmissionFailed(protectedFrame, stationRetryCounters);
    if (recoveryProcedure->isRtsFrameRetryLimitReached(protectedFrame)) {
        inProgressFrames->dropFrame(protectedFrame);
        delete protectedFrame;
    }
}

void Dcf::originatorProcessTransmittedFrame(Ieee80211Frame* transmittedFrame)
{
    if (transmittedFrame->getReceiverAddress().isMulticast())
        recoveryProcedure->multicastFrameTransmitted(stationRetryCounters);
    if (transmittedFrame->getType() == ST_DATA || dynamic_cast<Ieee80211ManagementFrame *>(transmittedFrame))
        ackHandler->processTransmittedNonQoSFrame(check_and_cast<Ieee80211DataOrMgmtFrame *>(transmittedFrame));
    else
        ;
}

void Dcf::originatorProcessReceivedFrame(Ieee80211Frame* frame, Ieee80211Frame* lastTransmittedFrame)
{
    if (frame->getType() == ST_ACK) {
        recoveryProcedure->ackFrameReceived(check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame), stationRetryCounters);
        ackHandler->processReceivedAck(check_and_cast<Ieee80211ACKFrame *>(frame), check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
        inProgressFrames->dropFrame(check_and_cast<Ieee80211DataOrMgmtFrame*>(lastTransmittedFrame));
    }
    else if (frame->getType() == ST_RTS)
        ; // void
    else if (frame->getType() == ST_CTS)
        recoveryProcedure->ctsFrameReceived(stationRetryCounters);
    else
        throw cRuntimeError("Unknown frame type");
}

void Dcf::originatorProcessFailedFrame(Ieee80211DataOrMgmtFrame* failedFrame)
{
    ASSERT(failedFrame->getType() != ST_DATA_WITH_QOS);
    ASSERT(ackHandler->getAckStatus(failedFrame) == AckHandler::Status::WAITING_FOR_NORMAL_ACK);
    EV_INFO << "Data/Mgmt frame transmission failed\n";
    recoveryProcedure->dataOrMgmtFrameTransmissionFailed(failedFrame, stationRetryCounters);
    if (recoveryProcedure->isDataOrMgmtFrameRetryLimitReached(failedFrame)) {
        inProgressFrames->dropFrame(failedFrame);
        delete failedFrame;
    }
}

void Dcf::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
 {
    ASSERT(frame->getControlInfo() == nullptr);
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
}

Dcf::~Dcf()
{
    delete pendingQueue;
    delete ackHandler;
    delete recipientAckProcedure;
    delete inProgressFrames;
    delete ctsProcedure;
}

} // namespace ieee80211
} // namespace inet
