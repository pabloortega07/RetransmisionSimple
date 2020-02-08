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

#ifndef __MIXIM_APPRSU_H_
#define __MIXIM_APPRSU_H_

#include <omnetpp.h>
#include <WaveShortMessage_m.h>
#include <WaveAppToMac1609_4Interface.h>
#include <map>
#include <Consts80211p.h>
#include <ChannelAccess.h>




#ifndef DBG
#define DBG EV
#endif

/**
 * TODO - Generated class
 */
class AppRSU : public cSimpleModule {

  public:

    ~AppRSU();
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  protected:

    virtual void handleLowerMsg (cMessage* msg);
    virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0);
    virtual void sendWSM(WaveShortMessage* wsm);
    virtual void onBeacon(WaveShortMessage* wsm);

  protected:

    int lowerLayerIn;
    int lowerLayerOut;
    int beaconLengthBits;
    int beaconPriority;
    Coord curPosition;
    int myId;

    WaveAppToMac1609_4Interface* myMac;

};

#endif
