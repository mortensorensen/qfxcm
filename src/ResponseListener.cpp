#include "stdafx.h"
#include <math.h>
#include <sstream>
#include <iomanip>
#include "Offer.h"
#include "ResponseListener.h"
#include "Helpers.h"

ResponseListener::ResponseListener(IO2GSession *session)
{
    mSession = session;
    mSession->addRef();
    mRefCount = 1;
    mResponseEvent = CreateEvent(0, FALSE, FALSE, 0);
    mRequestID = "";
    mOrderID = "";
    mResponse = NULL;
    std::cout.precision(2);
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
    auto readerFactory = session->getResponseReaderFactory();
    if (!readerFactory)
        O("failed to create reader factory\n"); R;
    
    // to bulk insert instead
    O("on offer ...\n");
    
    auto offersResponseReader = readerFactory->createOffersTableReader(response);
    if (!offersResponseReader) {
        for (I i = 0;i < offersResponseReader->size(); ++i) {
            auto offerRow = offersResponseReader->getRow(i);
            Offer *offer = mOffers->findOffer(offerRow->getOfferID());
            if (offer) {
                if (offerRow->isTimeValid() && offerRow->isBidValid() && offerRow->isAskValid()) {
                    offer->setDate(offerRow->getTime());
                    offer->setBid(offerRow->getBid());
                    offer->setAsk(offerRow->getAsk());
                }
            } else {
                offer = new Offer(offerRow->getOfferID(), offerRow->getInstrument(),
                                  offerRow->getDigits(), offerRow->getPointSize(),
                                  offerRow->getTime(), offerRow->getBid(), offerRow->getAsk());
                mOffers->addOffer(offer);
            }
            if (!sInstrument || strlen(sInstrument) == 0 || strcmp(offerRow->getInstrument(), sInstrument) == 0) {
                K keys = ktn(KS, 4);
                kS(keys)[0] = ss((S) "Date");
                kS(keys)[1] = ss((S) "Symbol");
                kS(keys)[2] = ss((S) "Bid");
                kS(keys)[3] = ss((S) "Ask");
                
                K vals = knk(4,
                             toKTime(offer->getDate()),
                             ss((S) offer->getInstrument()),
                             offer->getBid(),
                             offer->getAsk());
                
//                consumeEvent("onoffer", xD(keys, vals));
                O("ResponseListener: onoffer\n");
            }
        }
    }
}

void ResponseListener::onLevel2MarketData(IO2GSession *session, IO2GResponse *response, const char *sInstrument)
{
    auto readerFactory = session->getResponseReaderFactory();
    if (!readerFactory)
        O("failed to create reader factory\n"); R;
    
    auto lvl2ResponseReader = readerFactory->createLevel2MarketDataReader(response);
    if (lvl2ResponseReader != NULL) {
        for (I i = 0, ii = lvl2ResponseReader->getPriceQuotesCount(); i < ii; ++i) {
            // todo...
            for (I j = 0, jj = lvl2ResponseReader->getPricesCount(i); j < jj; ++j) {
                // todo...
            }
        }
    }
    
//    consumeEvent("onlevel2data", 0);
    O("ResponseListener: onlevel2\n");
}
