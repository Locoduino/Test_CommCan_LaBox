

#include "CanMsg.h"
#include "LaBoxCmd.h"

LaBoxCmd::LaBoxCmd(uint8_t id, uint8_t hash) :

                                               mID(id),
                                               mHash(hash),
                                               mDebug(false),
                                               mPower(false)
{
}

LaBoxCmd::~LaBoxCmd()
{
    if (mDebug)
        Serial.println(F("### Destroying controller"));
}

void LaBoxCmd::setup()
{
}

bool LaBoxCmd::setPower(bool power)
{
    CanMsg::sendMsg(0, mID, mHash, 0xFE, power);
    mPower = power;
    return power;
}

bool LaBoxCmd::setThrottle(Loco *loco)
{
    CanMsg::sendMsg(0, mID, mHash, 0xF0, loco->address & 0xFF00, loco->address & 0x00FF, loco->speed, loco->direction);
    return true;
}

bool LaBoxCmd::toggleThrottleDir(Loco *loco)
{
    loco->direction = !loco->direction;
    CanMsg::sendMsg(0, mID, mHash, 0xF0, loco->address & 0xFF00, loco->address & 0x00FF, loco->speed, loco->direction);
    return true;
}

bool LaBoxCmd::setFunction(Loco *loco, byte idx)
{
    CanMsg::sendMsg(0, mID, mHash, 0xF1, loco->address & 0xFF00, loco->address & 0x00FF, idx, loco->fn[idx]);
    return true;
}

bool LaBoxCmd::toggleFunction(Loco *loco, byte idx)
{
    loco->fn[idx] = !loco->fn[idx];
    CanMsg::sendMsg(0, mID, mHash, 0xF1, loco->address & 0xFF00, loco->address & 0x00FF, idx, loco->fn[idx]);
    return true;
}

bool LaBoxCmd::emergency()
{
    CanMsg::sendMsg(0, mID, mHash, 0xFF);
    return true;
}
