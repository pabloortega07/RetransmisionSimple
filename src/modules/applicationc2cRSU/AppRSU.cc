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

#include "AppRSU.h"

Define_Module(AppRSU);

void AppRSU::initialize() {

    lowerLayerIn = findGate("lowerLayerIn");
    lowerLayerOut = findGate("lowerLayerOut");
    myMac = FindModule<WaveAppToMac1609_4Interface*>::findSubModule(
                        getParentModule());
    assert(myMac);

    myId = getParentModule()->getIndex();
    beaconLengthBits = par("beaconLengthBits").longValue();
    beaconPriority = par("beaconPriority").longValue();


}

void AppRSU::handleMessage(cMessage *msg) {
    if(msg->getArrivalGateId()==lowerLayerIn) {
        handleLowerMsg(msg);
    } else {
        opp_error("No LowerMessage?? Check configuration.");
    }

}

void AppRSU::handleLowerMsg(cMessage* msg) {

    WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
    ASSERT(wsm);

    if (std::string(wsm->getName()) == "beacon") {
        onBeacon(wsm);
    }
    else {
        DBG << "unknown message (" << wsm->getName() << ")  received\n";
    }
    delete(msg);
}


void AppRSU::onBeacon(WaveShortMessage* wsm) {
    sendWSM(prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1));
 }


WaveShortMessage*  AppRSU::prepareWSM(std::string name, int lengthBits, t_channel channel, int priority, int rcvId, int serial) {
    WaveShortMessage* wsm =     new WaveShortMessage(name.c_str());
    wsm->addBitLength(beaconLengthBits);

    switch (channel) {
        case type_SCH: wsm->setChannelNumber(Channels::SCH1); break; //will be rewritten at Mac1609_4 to actual Service Channel. This is just so no controlInfo is needed
        case type_CCH: wsm->setChannelNumber(Channels::CCH); break;
    }
    wsm->setPsid(0);
    wsm->setPriority(priority);
    wsm->setWsmVersion(1);
    wsm->setTimestamp(simTime());
    wsm->setSenderAddress(myId);
    wsm->setRecipientAddress(rcvId);
    wsm->setSenderPos(curPosition);
    wsm->setSerial(serial);

    if (name == "beacon") {
        DBG << "Creating Beacon with Priority " << priority << " at Applayer at " << wsm->getTimestamp() << std::endl;
    }

    return wsm;
}





void AppRSU::sendWSM(WaveShortMessage* wsm) {
    send(wsm,lowerLayerOut);
}


void AppRSU::finish() {

}

AppRSU::~AppRSU() {

}
