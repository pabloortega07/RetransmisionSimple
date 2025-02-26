//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.mixim.modules.mac.ieee80211p;

import org.mixim.base.modules.BaseMacLayer;

//
// Manages timeslots for CCH and SCH listening and sending.
//
// @author Christopher Saloman
// @author David Eckhoff : rewrote complete model
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see PhyLayer80211p
//
simple Mac1609_4 extends BaseMacLayer
{
    parameters:
        @class(Mac1609_4);

        // cycle between an SCH and the CCH (or stay on CCH all the time)
        bool useServiceChannel = default(false);

        // the service channel this maclayer listens on
        int serviceChannel = default(1);

        // debug switch
        bool debug = default(false);

        // length of MAC header, header is already added at 1609_4
        headerLength @unit(bit) = default(0 bit);

        // bit rate 
        double bitrate @unit(bps) = default(18 Mbps);

        // little asynchronization between cars
        double syncOffset @unit(s) = default(0.0003s);

        //tx power [mW]
        double txPower @unit(mW);

        int queueSize = default(0);
}
