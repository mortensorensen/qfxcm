#include "stdafx.h"
#include "SessionStatusListener.h"
#include "k.h"

K consume_event2(const std::string &fun, K x) {
    K ret = k(0, (S)fun.c_str(), x, (K)0);
    
    if (!ret)
        O("Broken socket\n");
    else if (ret->t == -128)
        O("Error calling %s: %s\n", fun.c_str(), ret->s);
    
    r0(ret);
    R 0;
}


/** Constructor. */
SessionStatusListener::SessionStatusListener(IO2GSession *session,
                                             bool printSubsessions,
                                             const char *sessionID,
                                             const char *pin) {
    
    mSessionID = sessionID != 0 ? sessionID : "";
    mPin = pin != 0 ? pin : "";
    mSession = session;
    mSession->addRef();
    reset();
    mPrintSubsessions = printSubsessions;
    mRefCount = 1;
    mSessionEvent = CreateEvent(0, FALSE, FALSE, 0);
}

/** Destructor. */
SessionStatusListener::~SessionStatusListener() {
    mSession->release();
    mSessionID.clear();
    mPin.clear();
    CloseHandle(mSessionEvent);
}

/** Increase reference counter. */
long SessionStatusListener::addRef() {
    return InterlockedIncrement(&mRefCount);
}

/** Decrease reference counter. */
long SessionStatusListener::release() {
    long rc = InterlockedDecrement(&mRefCount);
    if (rc == 0)
        delete this;
    return rc;
}

void SessionStatusListener::reset() {
    mConnected = false;
    mDisconnected = false;
    mError = false;
}

/** Callback called when login has been failed. */
void SessionStatusListener::onLoginFailed(const char *error) {
    O("Login error: %s\n", error);
    mError = true;
}

/** Callback called when session status has been changed. */
void SessionStatusListener::onSessionStatusChanged(IO2GSessionStatus::O2GSessionStatus status) {
    switch (status) {
        case IO2GSessionStatus::Disconnected:
            O("status::disconnected\n");
            mConnected = false;
            mDisconnected = true;
            SetEvent(mSessionEvent);
            break;
        case IO2GSessionStatus::Connecting:
            O("status::connecting\n");
            break;
        case IO2GSessionStatus::TradingSessionRequested: {
            O("status::trading session requested\n");
            O2G2Ptr<IO2GSessionDescriptorCollection> descriptors =
                mSession->getTradingSessionDescriptors();
            bool found = false;
            if (descriptors) {
                if (mPrintSubsessions)
                    O("descriptors available:\n");
                DO(descriptors->size(),
                   O2G2Ptr<IO2GSessionDescriptor> descriptor = descriptors->get(i);
                   if (mPrintSubsessions)
                       O("  id:='%s' name='%s' description:='%s' requires pin:='%s'\n",
                         descriptor->getID(),
                         descriptor->getName(),
                         descriptor->getDescription(),
                         descriptor->requiresPin() ? "true" : "false");

                   if (mSessionID == descriptor->getID()) {
                       found = true;
                       break;
                   }
                )
            }

            if (!found)
                onLoginFailed("The specified sub session identifier is not found");
            else
                mSession->setTradingSession(mSessionID.c_str(), mPin.c_str());
        } break;
        case IO2GSessionStatus::Connected:
            O("status::connected\n");
            consume_event2("onconnect", (K)0);
            mConnected = true;
            mDisconnected = false;
            SetEvent(mSessionEvent);
            break;
        case IO2GSessionStatus::Reconnecting:
            O("status::reconnecting\n");
            break;
        case IO2GSessionStatus::Disconnecting:
            O("status::disconnecting\n");
            break;
        case IO2GSessionStatus::SessionLost:
            O("status::connection lost\n");
            break;
        default:
            break;
    }
}

/** Check whether error happened. */
bool SessionStatusListener::hasError() const { return mError; }

/** Check whether session is connected */
bool SessionStatusListener::isConnected() const { return mConnected; }

/** Check whether session is disconnected */
bool SessionStatusListener::isDisconnected() const { return mDisconnected; }

/** Wait for connection or error. */
bool SessionStatusListener::waitEvents() {
    return WaitForSingleObject(mSessionEvent, _TIMEOUT) == 0;
}
