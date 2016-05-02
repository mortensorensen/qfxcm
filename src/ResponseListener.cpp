#include "stdafx.h"
#include <math.h>
#include <sstream>
#include <iomanip>
#include "ResponseListener.h"
#include "Helpers.h"
#include "Offer.h"

ResponseListener::ResponseListener(IO2GSession *session, int sockets[])
{
    mSession = session;
    mSession->addRef();
    mRefCount = 1;
    mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
    mRequestID = "";
    mOrderID = "";
    mResponse = NULL;
    std::cout.precision(2);
    mSockets = sockets;
}

ResponseListener::~ResponseListener()
{
    if (mResponse)
        mResponse->release();
    mSession->release();
    CloseHandle(mResponseEvent);
}

/** Increase reference counter. */
long ResponseListener::addRef()
{
    return InterlockedIncrement(&mRefCount);
}

/** Decrease reference counter. */
long ResponseListener::release()
{
    long rc = InterlockedDecrement(&mRefCount);
    if (rc == 0)
        delete this;
    return rc;
}

/** Set request. */
void ResponseListener::setRequestID(const char *sRequestID)
{
    mRequestID = sRequestID;
    if (mResponse)
    {
        mResponse->release();
        mResponse = NULL;
    }
    ResetEvent(mResponseEvent);
}

void ResponseListener::setInstrument(const char *sInstrument)
{
    mInstrument = sInstrument;
}

bool ResponseListener::waitEvents()
{
    return WaitForSingleObject(mResponseEvent, _TIMEOUT) == 0;
}

/** Gets response.*/
IO2GResponse *ResponseListener::getResponse()
{
    if (mResponse)
        mResponse->addRef();
    return mResponse;
}

std::string ResponseListener::getOrderID()
{
    return mOrderID;
}

/** Request execution completed data handler. */
void ResponseListener::onRequestCompleted(const char *requestId, IO2GResponse *response)
{
    // TODO: Add callback here
    if (response && mRequestID == requestId)
    {
        mResponse = response;
        mResponse->addRef();
        if (response->getType() != CreateOrderResponse)
            SetEvent(mResponseEvent);
    }
}

/** Request execution failed data handler. */
void ResponseListener::onRequestFailed(const char *requestId , const char *error)
{
    if (mRequestID == requestId)
    {
        O("The request has been failed. ID: %s : %s\n", requestId, error);
        SetEvent(mResponseEvent);
    }
}

/** Request update data received data handler. */
void ResponseListener::onTablesUpdates(IO2GResponse *data)
{
    if (!data) return;
    
    switch (data->getType()) {
        case TablesUpdates:         onOffers(data);             break;
        case Level2MarketData:      onLevel2MarketData(data);   break;
        case GetAccounts:           onAccounts(data);           break;
        case GetTrades:             onTrades(data);             break;
        case GetOrders:             onOrders(data);             break;
        case GetClosedTrades:       onClosedTrades(data);       break;
        case GetMessages:           onMessages(data);           break;
        case GetLastOrderUpdate:    onLastOrderUpdate(data);    break;
        case GetSystemProperties:   onSystemProperties(data);   break;
        case MarginRequirementsResponse: onMarginRequirements(data); break;
        default: O("onTablesUpdates: Type: %i unknown\n", data->getType());
    }
}

void ResponseListener::onOffers(IO2GResponse *response)
{
    O2G2Ptr<IO2GResponseReaderFactory> readerFactory = mSession->getResponseReaderFactory();
    if (!readerFactory)
    {
        O("failed to create reader factory\n");
        return;
    }
    
    O2G2Ptr<IO2GOffersTableResponseReader> offersResponseReader = readerFactory->createOffersTableReader(response);
    if (!offersResponseReader) return;

//    K vals = knk(0);
    K vals;
    
    for (int i = 0; i < offersResponseReader->size(); ++i) {
        O2G2Ptr<IO2GOfferRow> offerRow = offersResponseReader->getRow(i);
        if (offerRow->isTimeValid() && offerRow->isBidValid() && offerRow->isAskValid()) {
//            jv(&vals, knk(4,
//                          toKTime(offerRow->getTime()),
//                          ks((S) offerRow->getInstrument()),
//                          kf(offerRow->getBid()),
//                          kf(offerRow->getAsk())));
            vals = knk(4,
                          toKTime(offerRow->getTime()),
                          ks((S) offerRow->getInstrument()),
                          kf(offerRow->getBid()),
                          kf(offerRow->getAsk()));
            writeToSocket(mSockets, "onoffer", vals);
        }
    }
    
//    writeToSocket(mSockets, "onoffer", vals);
}

void ResponseListener::onLevel2MarketData(IO2GResponse *response)
{
    auto readerFactory = mSession->getResponseReaderFactory();
    if (!readerFactory)
        O("failed to create reader factory\n"); R;
    
    auto lvl2ResponseReader = readerFactory->createLevel2MarketDataReader(response);
    if (lvl2ResponseReader) {
        for (int i = 0, ii = lvl2ResponseReader->getPriceQuotesCount(); i < ii; ++i) {
            // todo...
            for (int j = 0, jj = lvl2ResponseReader->getPricesCount(i); j < jj; ++j) {
                // todo...
            }
        }
    }
    
    O("ResponseListener: onlevel2\n");
}

void ResponseListener::onAccounts(IO2GResponse *response)
{
    O("ResponseListener: onAccounts\n");
}

void ResponseListener::onTrades(IO2GResponse *response)
{
    O("ResponseListener: onTrades\n");
}

void ResponseListener::onOrders(IO2GResponse *response)
{
    O("ResponseListener: onOrders\n");
}

void ResponseListener::onClosedTrades(IO2GResponse *response)
{
    O("ResponseListener: onClosedTrades\n");
}

void ResponseListener::onMessages(IO2GResponse *response)
{
    O("ResponseListener: onMessages\n");
}

void ResponseListener::onLastOrderUpdate(IO2GResponse *response)
{
    O("ResponseListener: onLastOrderProperties\n");
}

void ResponseListener::onSystemProperties(IO2GResponse *response)
{
    O("ResponseListener: onSystemProperties\n");
}

void ResponseListener::onMarginRequirements(IO2GResponse *response)
{
    O("ResponseListener: onMarginRequirements\n");
}
