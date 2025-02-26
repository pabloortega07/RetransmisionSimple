//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
// Copyright (C) 2012 Bastian Bloessl, Stefan Joerer, Michele Segata <{bloessl,joerer,segata}@ccs-labs.org>
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

#ifndef DECIDER80211p_H_
#define DECIDER80211p_H_
#include <omnetpp.h>
#include <BaseDecider.h>
#include <Consts80211p.h>
#include <Mac80211pToPhy11pInterface.h>
#include <Decider80211pToPhy80211pInterface.h>

#ifndef DBG_D11P
#define DBG_D11P EV
#endif
//#define DBG_D11P std::cerr << "[" << simTime().raw() << "] " << myPath << ".Dec "

/**
 * @brief
 * Based on Decider80211.h from Karl Wessel
 * and modifications by Christopher Saloman
 *
 * @author David Eckhoff
 *
 * @ingroup decider
 *
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class Decider80211p: public BaseDecider {
	public:
		enum Decider80211ControlKinds {
			NOTHING = 22100,
			BITERROR,       //the phy has recognized a bit error in the packet
			LAST_DECIDER_80211_CONTROL_KIND,
			RECWHILESEND
		};
	protected:
		// threshold value for checking a SNR-map (SNR-threshold)
		double snrThreshold;

		/** @brief The center frequency on which the decider listens for signals */
		double centerFrequency;

		double myBusyTime;
		double myStartTime;

		/** @brief Frame that the NIC card is currently trying to decode */
		AirFrame *curSyncFrame;

		std::string myPath;
		Decider80211pToPhy80211pInterface* phy11p;
		std::map<AirFrame*,int> signalStates;

		/** @brief enable/disable statistics collection for collisions
		 *
		 * For collecting statistics about collisions, we compute the Packet
		 * Error Rate for both SNR and SINR values. This might increase the
		 * simulation time, so if statistics about collisions are not needed,
		 * this variable should be set to false
		 */
		bool collectCollisionStats;
		/** @brief count the number of collisions */
		unsigned int collisions;



		/**
		 * @brief tell the outcome of a packetOk() call, which might be
		 * correctly decoded, discarded due to low SNR or discarder due
		 * to low SINR (i.e. collision)
		 */
		enum PACKET_OK_RESULT {DECODED, NOT_DECODED, COLLISION};

	protected:

		/**
		 * @brief Checks a mapping against a specific threshold (element-wise).
		 *
		 * @return	true	, if every entry of the mapping is above threshold
		 * 			false	, otherwise
		 *
		 *
		 */
		virtual DeciderResult* checkIfSignalOk(AirFrame* frame);

		virtual simtime_t processNewSignal(AirFrame* frame);

		/**
		 * @brief Processes a received AirFrame.
		 *
		 * The SNR-mapping for the Signal is created and checked against the Deciders
		 * SNR-threshold. Depending on that the received AirFrame is either sent up
		 * to the MAC-Layer or dropped.
		 *
		 * @return	usually return a value for: 'do not pass it again'
		 */
		virtual simtime_t processSignalEnd(AirFrame* frame);

		/** @brief computes if packet is ok or has errors*/
		enum PACKET_OK_RESULT packetOk(double snirMin, double snrMin, int lengthMPDU, double bitrate);

		/**
		 * @brief Calculates the RSSI value for the passed ChannelSenseRequest.
		 *
		 * This method is called by BaseDecider when it answers a ChannelSenseRequest
		 * and can be overridden by sub classing Deciders.
		 *
		 * Returns the maximum RSSI value inside the ChannelSenseRequest time
		 * interval and the channel the Decider currently listens to.
		 */
		virtual double calcChannelSenseRSSI(simtime_t_cref min, simtime_t_cref max);

		/**
		 * @brief Calculates a SNR-Mapping for a Signal.
		 *
		 * This method works as the calculateSnrMapping of the BaseDecider class,
		 * but it return the mapping for both SNR and SINR. This method is used
		 * to determine the frame reception probability for both SNR and SINR
		 * values. In this way we can determine (still probabilistically) if
		 * a frame has been dropped due to low signal power or due to a collision
		 *
		 */
		virtual void calculateSinrAndSnrMapping(AirFrame* frame, Mapping **sinrMap, Mapping **snrMap);

		/**
		 * @brief Calculates a RSSI-Mapping (or Noise-Strength-Mapping) for a
		 * Signal.
		 *
		 * This method is taken from the BaseDecider and changed in order to
		 * compute the RSSI mapping taking into account only thermal noise
		 *
		 * This method can be used to calculate a RSSI-Mapping in case the parameter
		 * exclude is omitted OR to calculate a Noise-Strength-Mapping in case the
		 * AirFrame of the received Signal is passed as parameter exclude.
		 */
		Mapping* calculateNoiseRSSIMapping(simtime_t_cref start, simtime_t_cref end, AirFrame *frame);

	public:

		/**
		 * @brief Initializes the Decider with a pointer to its PhyLayer and
		 * specific values for threshold and sensitivity
		 */
		Decider80211p(DeciderToPhyInterface* phy,
		              double sensitivity,
		              double centerFrequency,
		              int myIndex = -1,
//		              bool collectCollisionStatistics = true,
		              bool debug = false):
			BaseDecider(phy, sensitivity, myIndex, debug),
			centerFrequency(centerFrequency),
			myBusyTime(0),
			myStartTime(simTime().dbl()),
			curSyncFrame(0),
			collectCollisionStats(true),
			collisions(0) {
			phy11p = dynamic_cast<Decider80211pToPhy80211pInterface*>(phy);
			assert(phy11p);

		}

		void setPath(std::string myPath) {
			this->myPath = myPath;
		}

		bool cca(simtime_t_cref, AirFrame*);
		int getSignalState(AirFrame* frame);
		virtual ~Decider80211p();

		void changeFrequency(double freq);

		void setChannelIdleStatus(bool isIdle);

		/**
		 * @brief invoke this method when the phy layer is also finalized,
		 * so that statistics recorded by the decider can be written to
		 * the output file
		 */
		virtual void finish();

};

#endif /* DECIDER80211p_H_ */
