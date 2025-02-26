//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "TurtleMobility.h"
#include "FWMath.h"


Define_Module(TurtleMobility);


/**
 * Reads the parameters.
 * If the host is not stationary it calculates a random position and
 * schedules a timer to trigger the first movement
 */
void TurtleMobility::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    debugEV << "initializing TurtleMobility stage " << stage << endl;

    if(stage == 0)
    {
        turtleScript = par("turtleScript");
        nextStatement = turtleScript->getFirstChild();

        move.setSpeed(1);
        angle = 0;
        borderPolicy = REFLECT;

        // a dirty trick to extract starting position out of the script
        // (start doing it, but then rewind to the beginning)

	// does not really work
	// we need to  set the startPos initially for which there is no option in
	// turtlemobility
	// -> always starting with random position... (Daniel)
        //resumeScript();
        //nextStatement = turtleScript->getFirstChild();
        //while (!loopVars.empty()) loopVars.pop();

        // WATCH for member 'speed' of the Move is disabled at the moment
        // because the member is not accessible directly anymore
        //WATCH(move.speed);
        WATCH(angle);
        WATCH(borderPolicy);
    }
    else if(stage == 1){
    	if(!world->use2D()) {
			opp_warning("This mobility module does not yet support 3 dimensional movement."\
						"Movements will probably be incorrect.");
		}

		targetPos = move.getStartPos();
		stepTarget = move.getStartPos();
		targetTime = simTime();
    }
}

void TurtleMobility::setTargetPosition()
{
    resumeScript();

    move.setStart(move.getStartPos(), simTime());
}

void TurtleMobility::fixIfHostGetsOutside()
{
	debugEV << "stepTarget: " << stepTarget.info() << " targetPos: " << targetPos.info() << " stepSize: " << stepSize.info()
       << " angle: " << angle << endl;

    if( handleIfOutside(borderPolicy, stepTarget, targetPos, stepSize, angle) ){

    	debugEV << "stepTarget: " << stepTarget.info() << " targetPos: " << targetPos.info() << " stepSize: " << stepSize.info()
	   << " angle: " << angle << endl;

    }
}

/**
 * Will set a new targetTime and targetPos
 */
void TurtleMobility::resumeScript()
{
    if (!nextStatement)
    {
	move.setSpeed(0);
	debugEV << "no statement found -> not moving!\n";
        return;
    }

    simtime_t now = targetTime;

    // interpret statement
    while (nextStatement && targetTime==now)
    {
        executeStatement(nextStatement);
        gotoNextStatement();
    }
}

void TurtleMobility::executeStatement(cXMLElement *stmt)
{
    const char *tag = stmt->getTagName();

    debugEV << "doing <" << tag << ">\n";

    if (!strcmp(tag,"repeat"))
    {
        const char *nAttr = stmt->getAttribute("n");
        long n = -1;  // infinity -- that's the default
        if (nAttr)
        {
            n = (long) getValue(nAttr);
            if (n<0)
                error("<repeat>: negative repeat count at %s", stmt->getSourceLocation());
        }
        loopVars.push(n);
    }


    else if (!strcmp(tag,"set"))
    {
        const char *speedAttr = stmt->getAttribute("speed");
        const char *angleAttr = stmt->getAttribute("angle");
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *bpAttr = stmt->getAttribute("borderPolicy");
        if (speedAttr)
            move.setSpeed(getValue(speedAttr));
        if (angleAttr)
            angle = getValue(angleAttr);
        if (xAttr)
            targetPos.x = (getValue(xAttr));
        if (yAttr)
            targetPos.y = (getValue(yAttr));
        if (move.getSpeed()<=0)
            error("<set>: speed is negative or zero at %s", stmt->getSourceLocation());
        if (bpAttr)
        {
            if (!strcmp(bpAttr,"reflect"))
                borderPolicy = REFLECT;
            else if (!strcmp(bpAttr,"wrap"))
                borderPolicy = WRAP;
            else if (!strcmp(bpAttr,"placerandomly"))
                borderPolicy = PLACERANDOMLY;

            else if (!strcmp(bpAttr,"error"))
                borderPolicy = RAISEERROR;
            else
                error("<set>: value for attribute borderPolicy is invalid, should be "
                      "'reflect', 'wrap', 'placerandomly' or 'error' at %s",
                      stmt->getSourceLocation());
        }
    }
    else if (!strcmp(tag,"forward"))
    {
        const char *dAttr = stmt->getAttribute("d");
        const char *tAttr = stmt->getAttribute("t");
        if (!dAttr && !tAttr)
            error("<forward>: must have at least attribute 't' or 'd' (or both) at %s", stmt->getSourceLocation());
        double d, t;
        if (tAttr && dAttr)
        {
            // cover distance d in time t (current speed is ignored)
            d = getValue(dAttr);
            t = getValue(tAttr);
        }
        else if (dAttr)
        {
            // travel distance d at current speed
            d = getValue(dAttr);
            t = d / move.getSpeed();
        }
        else // tAttr only
        {
            // travel for time t at current speed
            t = getValue(tAttr);
            d = move.getSpeed() * t;
        }
        if (t<0)
            error("<forward>: time (attribute t) is negative at %s", stmt->getSourceLocation());
        if (d<0)
            error("<forward>: distance (attribute d) is negative at %s", stmt->getSourceLocation());
        // FIXME handle zeros properly...
        targetPos.x = (targetPos.x + d * cos(PI * angle / 180));
        targetPos.y = (targetPos.y + d * sin(PI * angle / 180));
        targetTime += t;
    }
    else if (!strcmp(tag,"turn"))
    {
        const char *angleAttr = stmt->getAttribute("angle");
        if (!angleAttr)
            error("<turn>: required attribute 'angle' missing at %s", stmt->getSourceLocation());
        angle += getValue(angleAttr);
    }
    else if (!strcmp(tag,"wait"))
    {
        const char *tAttr = stmt->getAttribute("t");
        if (!tAttr)
            error("<wait>: required attribute 't' missing at %s", stmt->getSourceLocation());
        simtime_t t = getValue(tAttr);
        if (t<0)
            error("<wait>: time (attribute t) is negative (%s) at %s", SIMTIME_STR(t), stmt->getSourceLocation());
        targetTime += t;  // targetPos is unchanged
    }
    else if (!strcmp(tag,"moveto"))
    {
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *tAttr = stmt->getAttribute("t");

        if (xAttr)
            targetPos.x = (getValue(xAttr));
        if (yAttr)
            targetPos.y = (getValue(yAttr));
        // travel to targetPos at current speed, or get there in time t (ignoring current speed then)
        simtime_t t = tAttr ? getValue(tAttr) : move.getStartPos().distance(targetPos)/move.getSpeed();
        if (t<0)
            error("<wait>: time (attribute t) is negative at %s", stmt->getSourceLocation());
        targetTime += t;
    }
    else if (!strcmp(tag,"moveby"))
    {
        const char *xAttr = stmt->getAttribute("x");
        const char *yAttr = stmt->getAttribute("y");
        const char *tAttr = stmt->getAttribute("t");
        if (xAttr)
            targetPos.x = (targetPos.x + getValue(xAttr));
        if (yAttr)
            targetPos.y = (targetPos.y + getValue(yAttr));
        // travel to targetPos at current speed, or get there in time t (ignoring current speed then)
        simtime_t t = tAttr ? getValue(tAttr) : move.getStartPos().distance(targetPos)/move.getSpeed();
        if (t<0)
            error("<wait>: time (attribute t) is negative at %s", stmt->getSourceLocation());
        targetTime += t;
    }
}

double TurtleMobility::getValue(const char *s)
{
    // first, textually replace $MAXX and $MAXY with their actual values
    std::string str;
    if (strchr(s,'$'))
    {
        char strMaxX[32], strMaxY[32];
        char strMinX[]="0.0", strMinY[]="0.0";
        sprintf(strMaxX, "%g", playgroundSizeX()-1);
        sprintf(strMaxY, "%g", playgroundSizeY()-1);

        str = s;
        std::string::size_type pos;
        while ((pos = str.find("$MAXX")) != std::string::npos)
            str.replace(pos, sizeof("$MAXX")-1, strMaxX);
        while ((pos = str.find("$MAXY")) != std::string::npos)
            str.replace(pos, sizeof("$MAXY")-1, strMaxY);
        while ((pos = str.find("$MINX")) != std::string::npos)
            str.replace(pos, sizeof("$MINX")-1, strMinX);
        while ((pos = str.find("$MINY")) != std::string::npos)
            str.replace(pos, sizeof("$MINY")-1, strMinY);
        s = str.c_str();
    }

    // then use cDynamicExpression to evaluate the string
    try {
        cDynamicExpression expr;
        expr.parse(s);
        return expr.doubleValue(this);
    }
    catch (std::exception& e) {
        throw cRuntimeError(this, "wrong value '%s' around %s: %s", s,
                nextStatement->getSourceLocation(), e.what());
    }
}

void TurtleMobility::gotoNextStatement()
{
    // "statement either doesn't have a child, or it's a <repeat> and loop count is already pushed on the stack"
    ASSERT(!nextStatement->getFirstChild() || (!strcmp(nextStatement->getTagName(),"repeat") && !loopVars.empty()));

    if (nextStatement->getFirstChild() && (loopVars.top()!=0 || (loopVars.pop(),false)))   // !=0: positive or -1
    {
        // statement must be a <repeat> if it has children; repeat count>0 must be
        // on the stack; let's start doing the body.
        nextStatement = nextStatement->getFirstChild();
    }
    else if (!nextStatement->getNextSibling())
    {
        // no sibling -- either end of <repeat> body, or end of script
        ASSERT(nextStatement->getParentNode()==turtleScript ? loopVars.empty() : !loopVars.empty());
        if (!loopVars.empty())
        {
            // decrement and check loop counter
            if (loopVars.top()!=-1)  // -1 means infinity
                loopVars.top()--;
            if (loopVars.top()!=0)  // positive or -1
            {
                // go to beginning of <repeat> block again
                nextStatement = nextStatement->getParentNode()->getFirstChild();
            }
            else
            {
                // end of loop -- locate next statement after the <repeat>
                nextStatement = nextStatement->getParentNode();
                gotoNextStatement();
            }
        }
        else
        {
            // end of script
            nextStatement = NULL;
        }
    }
    else

    {
        // go to next statement (must exist -- see "if" above)
        nextStatement = nextStatement->getNextSibling();
    }
}


