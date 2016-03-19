#pragma once

/** Response listener class. */

class ResponseListener : public IO2GResponseListener
{
 public:
    ResponseListener(IO2GSession *session);

    /** Increase reference counter. */
    virtual long addRef();

    /** Decrease reference counter. */
    virtual long release();

    /** Set request ID. */
    void setRequestID(const char *sRequestID);

    /** Wait for request execution or error. */
    bool waitEvents();

    /** Get response.*/
    IO2GResponse *getResponse();

    std::string getOrderID();

    /** Request execution completed data handler. */
    virtual void onRequestCompleted(const char *requestId, IO2GResponse *response = 0);

    /** Request execution failed data handler. */
    virtual void onRequestFailed(const char *requestId , const char *error);

    /** Request update data received data handler. */
    virtual void onTablesUpdates(IO2GResponse *data);

 private:
    long mRefCount;
    /** Session object. */
    IO2GSession *mSession;
    /** Request we are waiting for. */
    std::string mRequestID;
    /** Response Event handle. */
    HANDLE mResponseEvent;

    /** Order ID. */
    std::string mOrderID;

    /** State of last request. */
    IO2GResponse *mResponse;

 protected:
    /** Destructor. */
    virtual ~ResponseListener();

};

