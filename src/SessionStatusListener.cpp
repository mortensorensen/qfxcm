#include "stdafx.h"
#include "SessionStatusListener.h"
#include "Helpers.h"


/** Constructor. */
SessionStatusListener::SessionStatusListener(IO2GSession *session,
                                             bool printSubsessions,
                                             const char *sessionID,
                                             const char *pin,
                                             int sockets[])
{
    mSessionID = sessionID != 0 ? sessionID : "";
    mPin = pin != 0 ? pin : "";
    mSession = session;
    mSession->addRef();
    reset();
    mPrintSubsessions = printSubsessions;
    mRefCount = 1;
    mSessionEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (!sockets)
        throw;
    else
        mSockets = sockets;
}

/** Destructor. */
SessionStatusListener::~SessionStatusListener()
{
    mSession->release();
    mSessionID.clear();
    mPin.clear();
    CloseHandle(mSessionEvent);
}

/** Increase reference counter. */
long SessionStatusListener::addRef()
{
    return InterlockedIncrement(&mRefCount);
}

/** Decrease reference counter. */
long SessionStatusListener::release()
{
    long rc = InterlockedDecrement(&mRefCount);
    if (rc == 0)
        delete this;
    return rc;
}

void SessionStatusListener::reset()
{
    mConnected = false;
    mDisconnected = false;
    mError = false;
}

/** Callback called when login has been failed. */
void SessionStatusListener::onLoginFailed(const char *error)
{
    writeToSocket(mSockets, "loginfailed");
//    consumeEvent("loginfailed", kp((S)error));
    mError = true;
}

/** Callback called when session status has been changed. */
void SessionStatusListener::onSessionStatusChanged(IO2GSessionStatus::O2GSessionStatus status)
{
    switch (status) {
        case IO2GSessionStatus::Disconnected:
            writeToSocket(mSockets, "disconnected");
            mConnected = false;
            mDisconnected = true;
            SetEvent(mSessionEvent);
            break;
        case IO2GSessionStatus::Connecting:
            writeToSocket(mSockets, "connecting");
            break;
        case IO2GSessionStatus::TradingSessionRequested: {
            writeToSocket(mSockets, "tradingsessionrequested");
            O2G2Ptr<IO2GSessionDescriptorCollection> descriptors =
                mSession->getTradingSessionDescriptors();
            bool found = false;
            if (descriptors) {
                if (mPrintSubsessions)
                    O("descriptors available:\n");
                
                for (int i = 0; i < descriptors->size(); i++) {
                    auto descriptor = descriptors->get(i);
                    if (mPrintSubsessions) {
                        O("  id:='%s' name='%s' description:='%s' requires pin:='%s'\n",
                          descriptor->getID(),
                          descriptor->getName(),
                          descriptor->getDescription(),
                          descriptor->requiresPin() ? "yes" : "no");
                    }
                    
                    if (mSessionID == descriptor->getID()) {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                onLoginFailed("The specified sub session identifier is not found");
            else
                mSession->setTradingSession(mSessionID.c_str(), mPin.c_str());
        } break;
        case IO2GSessionStatus::Connected:
            writeToSocket(mSockets, "connected");
            mConnected = true;
            mDisconnected = false;
            SetEvent(mSessionEvent);
            break;
        case IO2GSessionStatus::Reconnecting:
            writeToSocket(mSockets, "reconnecting");
            break;
        case IO2GSessionStatus::Disconnecting:
            writeToSocket(mSockets, "disconnecting");
            break;
        case IO2GSessionStatus::SessionLost:
            writeToSocket(mSockets, "sessionlost");
            mSession->unsubscribeSessionStatus(this);
            release();
            mSession->release();
            break;
        default:
            break;

    }
}

/** Check whether error happened. */
bool SessionStatusListener::hasError() const
{
    return mError;
}

/** Check whether session is connected */
bool SessionStatusListener::isConnected() const
{
    return mConnected;
}

/** Check whether session is disconnected */
bool SessionStatusListener::isDisconnected() const
{
    return mDisconnected;
}

/** Wait for connection or error. */
bool SessionStatusListener::waitEvents()
{
    return WaitForSingleObject(mSessionEvent, _TIMEOUT) == 0;
}
