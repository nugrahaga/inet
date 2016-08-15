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
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

namespace inet {
namespace ieee80211 {

Define_Module(RateSelection);

void RateSelection::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        getContainingNicModule(this)->subscribe(NF_MODESET_CHANGED, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        const char *rateControlModule = par("rateControlModule");
        if (strlen(rateControlModule) != 0)
            dataOrMgmtRateControl = dynamic_cast<IRateControl*>(getModuleByPath(rateControlModule));
        double multicastFrameBitrate = par("multicastFrameBitrate");
        multicastFrameMode = (multicastFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(multicastFrameBitrate));
        double dataFrameBitrate = par("dataFrameBitrate");
        dataFrameMode = (dataFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(dataFrameBitrate));
        double mgmtFrameBitrate = par("mgmtFrameBitrate");
        mgmtFrameMode = (mgmtFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(mgmtFrameBitrate));
        double controlFrameBitrate = par("controlFrameBitrate");
        controlFrameMode = (controlFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(controlFrameBitrate));
        double responseAckFrameBitrate = par("responseAckFrameBitrate");
        responseAckFrameMode = (responseAckFrameBitrate == -1) ? nullptr : modeSet->getMode(bps(responseAckFrameBitrate));
        double responseCtsFrameBitrate = par("responseCtsFrameBitrate");
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
    }
}

const IIeee80211Mode* RateSelection::getMode(Ieee80211Frame* frame)
{
    IIeee80211Mode* mode = check_and_cast<IIeee80211Mode*>(frame->getControlInfo());
    return mode;
}

//
// If a CTS or ACK control response frame is carried in a non-HT PPDU, the primary rate is defined to
// be the highest rate in the BSSBasicRateSet parameter that is less than or equal to the rate (or non-HT
// reference rate; see 9.7.9) of the previous frame. If no rate in the BSSBasicRateSet parameter meets
// these conditions, the primary rate is defined to be the highest mandatory rate of the attached PHY
// that is less than or equal to the rate (or non-HT reference rate; see 9.7.9) of the previous frame. The
// STA may select an alternate rate according to the rules in 9.7.6.5.4. The STA shall transmit the
// non-HT PPDU CTS or ACK control response frame at either the primary rate or the alternate rate, if
// one exists.
//
const IIeee80211Mode* RateSelection::computeResponseAckFrameMode(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    // TODO: BSSBasicRateSet
    return getMode(dataOrMgmtFrame);
}

const IIeee80211Mode* RateSelection::computeResponseCtsFrameMode(Ieee80211RTSFrame *rtsFrame)
{
    // TODO: BSSBasicRateSet
    return getMode(rtsFrame);
}

const IIeee80211Mode* RateSelection::computeDataOrMgmtFrameMode(Ieee80211DataOrMgmtFrame* dataOrMgmtFrame)
{

}

const IIeee80211Mode* RateSelection::computeControlFrameMode(Ieee80211Frame* frame)
{

}

const IIeee80211Mode* RateSelection::computeMode(Ieee80211Frame* frame)
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame*>(frame))
        return computeDataOrMgmtFrameMode(dataOrMgmtFrame);
    else
        return computeControlFrameMode(frame);
}

void RateSelection::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method("receiveModeSetChangeNotification");
    if (signalID == NF_MODESET_CHANGED)
        modeSet = check_and_cast<Ieee80211ModeSet*>(obj);
}

void RateSelection::frameTransmitted(Ieee80211Frame* frame)
{
    auto receiverAddr = frame->getReceiverAddress();
    lastTransmittedFrameMode[receiverAddr] = getMode(frame);
}

} // namespace ieee80211
} // namespace inet
