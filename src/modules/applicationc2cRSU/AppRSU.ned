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

package org.mixim.modules.applicationc2cRSU;

//
// TODO auto-generated module
//
simple AppRSU
{
    parameters:
        int beaconLengthBits = default(1600bit) @unit(bit); //the length of a beacon packet
        int beaconPriority = default(3); //the access category of the beacon messages       
        @display("i=block/broadcast,red,30");
    gates:
        input lowerLayerIn;    // from MAC Layer
        output lowerLayerOut;  // to MAC Layer
        input lowerControlIn;
        output lowerControlOut;
}
