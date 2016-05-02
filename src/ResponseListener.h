#pragma once

/** Response listener class. */

class Offer;
class OfferCollection;

class ResponseListener : public IO2GResponseListener
{
 public:
    ResponseListener(IO2GSession *session, int sockets[]);

    /** Increase reference counter. */
    virtual long addRef();

    /** Decrease reference counter. */
    virtual long release();

    /** Set request ID. */
    void setRequestID(const char *sRequestID);

    void setInstrument(const char *sInstrument);
    
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

    void onOffers(IO2GResponse *response);
    void onLevel2MarketData(IO2GResponse *response);
    void onAccounts(IO2GResponse *response);
    void onTrades(IO2GResponse *response);
    void onOrders(IO2GResponse *response);
    void onClosedTrades(IO2GResponse *response);
    void onMessages(IO2GResponse *response);
    void onLastOrderUpdate(IO2GResponse *response);
    void onSystemProperties(IO2GResponse *response);
    void onMarginRequirements(IO2GResponse *response);
    
 private:
    long mRefCount;
    /** Session object. */
    IO2GSession *mSession;
    /** Request we are waiting for. */
    std::string mRequestID;
    std::string mInstrument;
    /** Response Event handle. */
    HANDLE mResponseEvent;

    /** Order ID. */
    std::string mOrderID;

    /** State of last request. */
    IO2GResponse *mResponse;
    
    OfferCollection *mOffers;
    
    int *mSockets;

 protected:
    /** Destructor. */
    virtual ~ResponseListener();

};

