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

#include "inet/common/INETUtils.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.h"

namespace inet {
namespace ieee80211 {

Define_Module(FrameSequenceHandler);

FrameSequenceHandler::~FrameSequenceHandler()
{
    cancelAndDelete(timeout);
}

void FrameSequenceHandler::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        timeout = new cMessage("startRxTimeout");
    }
}

void FrameSequenceHandler::handleMessage(cMessage* message)
{
    ASSERT(message == timeout || message == endReceptionTimeout);
    auto lastStep = context->getLastStep();
    switch (lastStep->getType()) {
        case IFrameSequenceStep::Type::RECEIVE:
            if (message == timeout && !callback->isReceptionInProgress())
                abortFrameSequence();
            break;
        case IFrameSequenceStep::Type::TRANSMIT:
            throw cRuntimeError("Received timeout while in transmit step");
        default:
            throw cRuntimeError("Unknown step type");
    }
}

void FrameSequenceHandler::processResponse(Ieee80211Frame* frame)
{
    ASSERT(callback != nullptr);
    Enter_Method("processResponse(\"%s\")", frame->getName());
    auto lastStep = context->getLastStep();
    switch (lastStep->getType()) {
        case IFrameSequenceStep::Type::RECEIVE: {
            IReceiveStep *receiveStep = check_and_cast<IReceiveStep*>(context->getLastStep());
            receiveStep->setFrameToReceive(frame);
            finishFrameSequenceStep();
            if (isSequenceRunning())
                startFrameSequenceStep();
            break;
        }
        case IFrameSequenceStep::Type::TRANSMIT:
            throw cRuntimeError("Received frame while current step is transmit");
        default:
            throw cRuntimeError("Unknown step type");
    }
}

void FrameSequenceHandler::transmissionComplete()
{
    Enter_Method("transmissionComplete()");
    if (isSequenceRunning()) {
        finishFrameSequenceStep();
        if (isSequenceRunning())
            startFrameSequenceStep();
    }
}

void FrameSequenceHandler::startFrameSequence(IFrameSequence *frameSequence, FrameSequenceContext *context, IFrameSequenceHandler::ICallback *callback)
{
    this->callback = callback;
    if (!isSequenceRunning()) {
        this->frameSequence = frameSequence;
        this->context = context;
        frameSequence->startSequence(context, 0);
        startFrameSequenceStep();
    }
    else
        throw cRuntimeError("Channel access granted while a frame sequence is running");
}


void FrameSequenceHandler::startFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    auto nextStep = frameSequence->prepareStep(context);
    // EV_INFO << "Frame sequence history:" << frameSequence->getHistory() << endl;
    if (nextStep == nullptr)
        finishFrameSequence(true);
    else {
        context->addStep(nextStep);
        switch (nextStep->getType()) {
            case IFrameSequenceStep::Type::TRANSMIT: {
                auto transmitStep = static_cast<TransmitStep *>(nextStep);
                EV_INFO << "Transmitting, frame = " << transmitStep->getFrameToTransmit() << "\n";
                // The allowable frame exchange sequence is defined by the rule frame sequence.
                // Except where modified by the pifs attribute, frames are separated by a SIFS.
                // (G.2 Basic sequences)
                callback->transmitFrame(transmitStep->getFrameToTransmit(), transmitStep->getIfs());
                // TODO: lifetime
                // if (auto dataFrame = dynamic_cast<Ieee80211DataFrame *>(transmitStep->getFrameToTransmit()))
                //    transmitLifetimeHandler->frameTransmitted(dataFrame);
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                // start reception timer, break loop if timer expires before reception is over
                auto receiveStep = static_cast<ReceiveStep *>(nextStep);
                scheduleAt(simTime() + receiveStep->getTimeout(), timeout);
                break;
            }
            default:
                throw cRuntimeError("Unknown frame sequence step type");
        }
    }
}

void FrameSequenceHandler::finishFrameSequenceStep()
{
    ASSERT(isSequenceRunning());
    auto lastStep = context->getLastStep();
    auto stepResult = frameSequence->completeStep(context);
    // EV_INFO << "Frame sequence history:" << frameSequence->getHistory() << endl;
    if (!stepResult) {
        lastStep->setCompletion(IFrameSequenceStep::Completion::REJECTED);
        cancelEvent(timeout);
        abortFrameSequence();
    }
    else {
        lastStep->setCompletion(IFrameSequenceStep::Completion::ACCEPTED);
        switch (lastStep->getType()) {
            case IFrameSequenceStep::Type::TRANSMIT: {
                auto transmitStep = static_cast<TransmitStep *>(lastStep);
                callback->originatorProcessTransmittedFrame(transmitStep->getFrameToTransmit());
                break;
            }
            case IFrameSequenceStep::Type::RECEIVE: {
                auto receiveStep = static_cast<ReceiveStep *>(lastStep);
                auto transmitStep = check_and_cast<ITransmitStep*>(context->getStepBeforeLast());
                callback->originatorProcessReceivedFrame(receiveStep->getReceivedFrame(), transmitStep->getFrameToTransmit());
                cancelEvent(timeout);
                break;
            }
            default:
                throw cRuntimeError("Unknown frame sequence step type");
        }
    }
}

void FrameSequenceHandler::finishFrameSequence(bool ok)
{
    EV_INFO << (ok ? "Frame sequence finished\n" : "Frame sequence aborted\n");
    delete context;
    delete frameSequence;
    context = nullptr;
    frameSequence = nullptr;
    callback->frameSequenceFinished();
    callback = nullptr;
}

void FrameSequenceHandler::abortFrameSequence()
{
    EV_INFO << "Frame sequence aborted\n";
    auto step = context->getLastStep();
    auto failedTxStep = check_and_cast<ITransmitStep*>(dynamic_cast<IReceiveStep*>(step) ? context->getStepBeforeLast() : step);
    auto frameToTransmit = failedTxStep->getFrameToTransmit();
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frameToTransmit))
        callback->originatorProcessFailedFrame(dataOrMgmtFrame);
    else if (auto rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frameToTransmit)) {
        auto rtsTxStep = dynamic_cast<RtsTransmitStep*>(failedTxStep);
        callback->originatorProcessRtsProtectionFailed(rtsTxStep->getProtectedFrame());
        delete rtsFrame;
    }
    else if (auto blockAckReq = dynamic_cast<Ieee80211BlockAckReq *>(frameToTransmit))
        delete blockAckReq;
    else ; // TODO: etc ?

    for (int i = 0; i < context->getNumSteps(); i++) {
        if (auto txStep = dynamic_cast<ITransmitStep*>(context->getStep(i)))
            if (txStep != failedTxStep)
                delete txStep->getFrameToTransmit();
    }
    finishFrameSequence(false);
}

} // namespace ieee80211
} // namespace inet
