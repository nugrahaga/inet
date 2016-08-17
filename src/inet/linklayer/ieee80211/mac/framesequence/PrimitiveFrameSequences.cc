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

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

void CtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *CtsFs::prepareStep(FrameSequenceContext *context)
{
    // TODO: Implement
    return nullptr;
}

bool CtsFs::completeStep(FrameSequenceContext *context)
{
    // TODO: Implement
    return false;
}

DataFs::DataFs(int ackPolicy) :
    ackPolicy(ackPolicy)
{
}

void DataFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *DataFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto frame = check_and_cast<Ieee80211DataFrame *>(context->getInProgressFrames()->getFrameToTransmit());
            frame->setType(ST_DATA_WITH_QOS); // FIXME: KLUDGE!
            frame->setAckPolicy(ackPolicy); // FIXME: KLUDGE!
            return new TransmitStep(frame, context->getIfs());
        }
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool DataFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        default:
            throw cRuntimeError("Unknown step");
    }
}

void ManagementFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *ManagementFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto mgmtFrame = check_and_cast<Ieee80211ManagementFrame *>(context->getInProgressFrames()->getFrameToTransmit());
            return new TransmitStep(mgmtFrame, context->getIfs());
        }
        case 1: {
            return new ReceiveStep(context->getAckProcedure()->getTimeout());
        }
        case 2:
            return nullptr;

        default:
            throw cRuntimeError("Unknown step");
    }
}

bool ManagementFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep*>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void AckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *AckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            return new ReceiveStep();
        case 1:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool AckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto receiveStep = check_and_cast<IReceiveStep*>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void RtsCtsFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *RtsCtsFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto dataOrMgmtFrame = check_and_cast<Ieee80211DataOrMgmtFrame *>(context->getInProgressFrames()->getFrameToTransmit());
            auto rtsFrame = context->getRtsProcedure()->buildRtsFrame(dataOrMgmtFrame);
            return new RtsTransmitStep(dataOrMgmtFrame, rtsFrame, context->getIfs());
        }
        case 1:
            return new ReceiveStep(context->getRtsProcedure()->getTimeout());
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool RtsCtsFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_CTS;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void FragFrameAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *FragFrameAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto frame = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(frame, context->getIfs());
        }
        case 1:
            return new ReceiveStep(context->getAckProcedure()->getTimeout());
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool FragFrameAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void LastFrameAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *LastFrameAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto frame = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(frame, context->getIfs());
        }
        case 1:
            return new ReceiveStep(context->getAckProcedure()->getTimeout());
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool LastFrameAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep *>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_ACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

void BlockAckReqBlockAckFs::startSequence(FrameSequenceContext *context, int firstStep)
{
    this->firstStep = firstStep;
    step = 0;
}

IFrameSequenceStep *BlockAckReqBlockAckFs::prepareStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0: {
            auto blockAckReq = context->getInProgressFrames()->getFrameToTransmit();
            return new TransmitStep(blockAckReq, context->getIfs());
        }
        case 1: {
            ITransmitStep *txStep = check_and_cast<ITransmitStep *>(context->getLastStep());
            auto blockAckReq = check_and_cast<Ieee80211BlockAckReq*>(txStep->getFrameToTransmit());
            return new ReceiveStep(context->getQoSContext()->ackPolicy->getBlockAckTimeout(blockAckReq));
        }
        case 2:
            return nullptr;
        default:
            throw cRuntimeError("Unknown step");
    }
}

bool BlockAckReqBlockAckFs::completeStep(FrameSequenceContext *context)
{
    switch (step) {
        case 0:
            step++;
            return true;
        case 1: {
            auto receiveStep = check_and_cast<IReceiveStep*>(context->getStep(firstStep + step));
            step++;
            return receiveStep->getReceivedFrame()->getType() == ST_BLOCKACK;
        }
        default:
            throw cRuntimeError("Unknown step");
    }
}

} // namespace ieee80211
} // namespace inet
