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
    if (data) {
        if (data->getType() == TablesUpdates) {
            onOffers(mSession, data, mInstrument.c_str());
        } else if (data->getType() == Level2MarketData) {
            onLevel2MarketData(mSession, data, mInstrument.c_str());
        }
    }
}

void ResponseListener::onOffers(IO2GSession *session, IO2GResponse *response, const char *sInstrument)
{
    O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
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

void ResponseListener::onLevel2MarketData(IO2GSession *session, IO2GResponse *response, const char *sInstrument)
{
    auto readerFactory = session->getResponseReaderFactory();
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
