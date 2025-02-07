//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#include <algorithm>

#include "../testUtils/asserts.h"
#include "../testUtils/OmnetTestBase.h"

#include "TraCITestApp.h"

Define_Module(TraCITestApp);

const simsignalwrap_t TraCITestApp::mobilityStateChangedSignal = simsignalwrap_t(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

void TraCITestApp::initialize(int stage) {
	BaseApplLayer::initialize(stage);
	if (stage == 0) {
		debug = par("debug");
		testNumber = par("testNumber");
		traci = TraCIMobilityAccess().get(getParentModule());
		findHost()->subscribe(mobilityStateChangedSignal, this);

		visitedEdges.clear();
		hasStopped = false;

		if (debug) std::cout << "TraCITestApp initialized with testNumber=" << testNumber << std::endl;
	}
}

void TraCITestApp::finish() {
}

void TraCITestApp::handleSelfMsg(cMessage *msg) {
}

void TraCITestApp::handleLowerMsg(cMessage* msg) {
	delete msg;
}

void TraCITestApp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate();
	}
}

void TraCITestApp::handlePositionUpdate() {
	const simtime_t t = simTime();
	const std::string roadId = traci->getRoadId();
	visitedEdges.insert(roadId);

	int testCounter = 0;

	if (testNumber == testCounter++) {
		if (t == 9) {
			assertTrue("(commandSetSpeed) vehicle is driving", traci->getSpeed() > 25);
		}
		if (t == 10) {
			traci->commandSetSpeedMode(0x00);
			traci->commandSetSpeed(0);
		}
		if (t == 11) {
			assertClose("(commandSetSpeed) vehicle has stopped", 0.0, traci->getSpeed());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			traci->commandChangeRoute("42", 9999);
			traci->commandChangeRoute("43", 9999);
		}
		if (t == 30) {
			assertTrue("(commandChangeRoute, 9999) vehicle avoided 42", visitedEdges.find("42") == visitedEdges.end());
			assertTrue("(commandChangeRoute, 9999) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
			assertTrue("(commandChangeRoute, 9999) vehicle took 44", visitedEdges.find("44") != visitedEdges.end());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			traci->commandChangeRoute("42", 9999);
			traci->commandChangeRoute("43", 9999);
		}
		if (t == 3) {
			traci->commandChangeRoute("42", -1);
			traci->commandChangeRoute("44", 9999);
		}
		if (t == 30) {
			assertTrue("(commandChangeRoute, -1) vehicle took 42", visitedEdges.find("42") != visitedEdges.end());
			assertTrue("(commandChangeRoute, -1) vehicle avoided 43", visitedEdges.find("43") == visitedEdges.end());
			assertTrue("(commandChangeRoute, -1) vehicle avoided 44", visitedEdges.find("44") == visitedEdges.end());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			assertClose("(commandDistanceRequest, air)", 859.4556417, traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), false));
			assertClose("(commandDistanceRequest, driving)", 845.93, traci->commandDistanceRequest(Coord(25,7030), Coord(883,6980), true));
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			traci->commandStopNode("43", 20, 0, 10, 30);
		}
		if (t == 30) {
			assertTrue("(commandStopNode) vehicle is at 43", roadId == "43");
			assertClose("(commandStopNode) vehicle is stopped", 0.0, traci->getSpeed());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			traci->getManager()->commandSetTrafficLightProgram("10", "myProgramRed");
		}
		if (t == 30) {
			assertTrue("(commandSetTrafficLightProgram) vehicle is at 31", roadId == "31");
			assertClose("(commandSetTrafficLightProgram) vehicle is stopped", 0.0, traci->getSpeed());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			traci->getManager()->commandSetTrafficLightPhaseIndex("10", 4);
		}
		if (t == 30) {
			assertTrue("(commandSetTrafficLightPhaseIndex) vehicle is at 31", roadId == "31");
			assertClose("(commandSetTrafficLightPhaseIndex) vehicle is stopped", 0.0, traci->getSpeed());
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			std::list<std::string> polys = traci->commandGetPolygonIds();
			assertEqual("(commandGetPolygonIds) number is 1", polys.size(), (size_t)1);
			assertEqual("(commandGetPolygonIds) id is correct", *polys.begin(), "poly0");
			std::string typeId = traci->commandGetPolygonTypeId("poly0");
			assertEqual("(commandGetPolygonTypeId) typeId is correct", typeId, "type0");
			std::list<Coord> shape = traci->commandGetPolygonShape("poly0");
			assertClose("(commandGetPolygonShape) shape x coordinate is correct", 130.0, shape.begin()->x);
			assertClose("(commandGetPolygonShape) shape y coordinate is correct", 81.65, shape.begin()->y);
		}
	}

	if (testNumber == testCounter++) {
		if (t == 1) {
			std::list<Coord> shape1 = traci->commandGetPolygonShape("poly0");
			assertClose("(commandGetPolygonShape) shape x coordinate is correct", 130.0, shape1.begin()->x);
			assertClose("(commandGetPolygonShape) shape y coordinate is correct", 81.65, shape1.begin()->y);
			std::list<Coord> shape2 = shape1;
			shape2.begin()->x = 135;
			shape2.begin()->y = 85;
			traci->commandSetPolygonShape("poly0", shape2);
			std::list<Coord> shape3 = traci->commandGetPolygonShape("poly0");
			assertClose("(commandSetPolygonShape) shape x coordinate was changed", 135.0, shape3.begin()->x);
			assertClose("(commandSetPolygonShape) shape y coordinate was changed", 85.0, shape3.begin()->y);
		}
	}

	if (testNumber == testCounter++) {
		if (t == 30) {
			std::list<Coord> points;
			points.push_back(Coord(100, 100));
			points.push_back(Coord(200, 100));
			points.push_back(Coord(200, 200));
			points.push_back(Coord(100, 200));
			traci->getManager()->commandAddPolygon("testPoly", "testType", TraCIScenarioManager::Color(1, 1, 1, 1), true, 1, points);
		}
		if (t == 31) {
			std::list<std::string> polys = traci->commandGetPolygonIds();
			assertEqual("(commandAddPolygon) number is 2", polys.size(), (size_t)2);
			assertTrue("(commandAddPolygon) ids contain added", std::find(polys.begin(), polys.end(), std::string("testPoly")) != polys.end());
			std::string typeId = traci->commandGetPolygonTypeId("testPoly");
			assertEqual("(commandAddPolygon) typeId is correct", typeId, "testType");
			std::list<Coord> shape = traci->commandGetPolygonShape("testPoly");
			assertClose("(commandAddPolygon) shape x coordinate is correct", 100.0, shape.begin()->x);
			assertClose("(commandAddPolygon) shape y coordinate is correct", 100.0, shape.begin()->y);
		}
	}

	if (testNumber == testCounter++) {
		if (t == 30) {
			std::list<std::string> lanes = traci->getManager()->commandGetLaneIds();
			assertTrue("(commandGetLaneIds) returns test lane", std::find(lanes.begin(), lanes.end(), "10_0") != lanes.end());
			std::list<Coord> shape = traci->getManager()->commandGetLaneShape("10_0");
			assertClose("(commandGetLaneShape) shape x coordinate is correct", 523.35, shape.begin()->x);
			assertClose("(commandGetLaneShape) shape y coordinate is correct", 80.83, shape.begin()->y);
		}
	}

	if (testNumber == testCounter++) {
		if (t == 30) {
			std::list<std::string> junctions = traci->getManager()->commandGetJunctionIds();
			assertTrue("(commandGetJunctionIds) returns test junction", std::find(junctions.begin(), junctions.end(), "1") != junctions.end());
			Coord pos = traci->getManager()->commandGetJunctionPosition("1");
			assertClose("(commandGetJunctionPosition) shape x coordinate is correct", 25.0, pos.x);
			assertClose("(commandGetJunctionPosition) shape y coordinate is correct", 75.0, pos.y);
		}
	}

	if (testNumber == testCounter++) {
		if (t == 30) {
			bool r = traci->getManager()->commandAddVehicle("testVehicle0", "vtype0", "route0", "25_0", 0, 70);
			assertTrue("(commandAddVehicle) command reports success", r);
		}
		if (t == 31) {
			std::map<std::string, cModule*>::const_iterator i = traci->getManager()->getManagedHosts().find("testVehicle0");
			bool r = (i != traci->getManager()->getManagedHosts().end());
			assertTrue("(commandAddVehicle) vehicle now driving", r);
			const cModule* mod = i->second;
			const TraCIMobility* traci2 = FindModule<TraCIMobility*>::findSubModule(const_cast<cModule*>(mod));
			assertTrue("(commandAddVehicle) vehicle driving at speed", traci2->getSpeed() > 25);
		}
	}
}

