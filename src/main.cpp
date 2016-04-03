/*
 * Wrapper for FXCM's ForexConnect API
 *
 * Author: Morten Sorensen
 *
 */

#include <config.h>

#include "stdafx.h"
#include "Helpers.h"
#include "ResponseListener.h"
#include "SessionStatusListener.h"
#include "CommonSources.h"
#include "Offer.h"
#include "MarketDataSnapshotAccumulator.h"

IO2GSession *session;
SessionStatusListener *sessionListener;
ResponseListener *responseListener;

extern "C" K connect(K host, K user, K password, K connection) {
    Q(host->t != -11 || user->t != -11 || password->t != -11 || connection->t != -11, "type");
    Q(sessionListener && sessionListener->isConnected(), "session");
    Q(strcasecmp(connection->s, "Real") != 0 &&
      strcasecmp(connection->s, "Demo") != 0,
      "connection");
    
    session = CO2GTransport::createSession();
    sessionListener = new SessionStatusListener(session, false, "", "");
    session->subscribeSessionStatus(sessionListener);
    
//    sessionListener->reset();
    session->login(user->s, password->s, host->s, connection->s);

    responseListener = new ResponseListener(session);
    session->subscribeResponse(responseListener);
    
    R 0;
}

extern "C" K disconnect(K ignore) {
    Q(!session || !sessionListener, "session");
    
    session->unsubscribeResponse(responseListener);
    responseListener->release();
    
    sessionListener->reset();
    session->logout();
    sessionListener->waitEvents();
    
    session->unsubscribeSessionStatus(sessionListener);
    sessionListener->release();

    session->release();
    session = NULL;
    
    R 0;
}

extern "C" K isconnected(K ignore) {
    R kb(sessionListener && sessionListener->isConnected());
}

extern "C" K gethistprices(K kInstrument, K kTimeframe, K kDtFrom, K kDtTo) {
    Q(!session || !sessionListener, "session");
    
    auto factory = session->getRequestFactory();
    Q(!factory, "factory");
    
    // Find timeframe by identifer
    auto timeframeCollection = factory->getTimeFrameCollection();
    auto timeframe = timeframeCollection->get(kTimeframe->s);
    Q(!timeframe, "timeframe");
    
    auto request =
        factory->createMarketDataSnapshotRequestInstrument(kInstrument->s, timeframe, timeframe->getQueryDepth());
    DATE dtFirst, dtFrom;
    oz(kDtTo, &dtFirst);
    oz(kDtFrom, &dtFrom);
    
    double tolerance = 0.0001;
    auto accumulator = std::make_unique<MarketDataSnapshotAccumulator>();
    
    // There is a limit for retured candles amount
    do {
        factory->fillMarketDataSnapshotRequestTime(request, dtFrom, dtFirst, false);
        responseListener->setRequestID(request->getRequestID());
        session->sendRequest(request);
        Q(!responseListener->waitEvents(), "timeout");
        
        // shift "to" bound to oldest datetime of returned data
        auto response = responseListener->getResponse();
        
        if (response && (response->getType() == MarketDataSnapshot)) {
            auto readerFactory = session->getResponseReaderFactory();
            
            O("RequestID='%s' completed\n", response->getRequestID());
            
            if (readerFactory) {
                auto reader =
                    readerFactory->createMarketDataSnapshotReader(response);
            
                accumulator->addReader(reader);
                
                // check data is valid
                Q(reader->size() == 0, "0 rows received");
                if (std::abs(dtFirst - reader->getDate(0)) <= tolerance)
                    break;
                
                dtFirst = reader->getDate(0); // earliest datetime of returned data
            }
        } else {
            break;
        }
    } while (dtFirst - dtFrom > tolerance);
    
    accumulator->getTable();
    
    R 0;
}

extern "C" K requestMarketData(K kInstrument)
{
    Q(kInstrument->t != -11, "type");
    responseListener->setInstrument(kInstrument->s);
    
    auto loginRules = session->getLoginRules();
    Q(!loginRules, "loginrules");

    O2G2Ptr<IO2GResponse> response = NULL;
    if (loginRules->isTableLoadedByDefault(Offers)) {
        response = loginRules->getTableRefreshResponse(Offers);
        O("table loaded by default\n");
        if (response) {
            O("response..\n");
            responseListener->onOffers(session, response, "");
        }
        
    } else {
        auto requestFactory = session->getRequestFactory();
        if (requestFactory) {
            O("entering b\n");
            auto offersRequest = requestFactory->createRefreshTableRequest(Offers);
            responseListener->setRequestID(offersRequest->getRequestID());
            session->sendRequest(offersRequest);
            
            Q(!responseListener->waitEvents(), "timout");
            response = responseListener->getResponse();
            if (response) {
                responseListener->onOffers(session, response, "");
                O("c\n");
            } else {
                O("d\n");
            }
        } else {
            O("no req factory\n");
        }
    }
    
    R 0;
}

extern "C" K createOrder(K kAccountId, K kOfferId, K kAmount, K kCustomId)
{
    Q(kAccountId->t != -11 || kOfferId->t != -7 || kAmount->t != -7 || kCustomId->t != -11, "type");
    Q(kAmount->j == 0, "amount");
    
    auto factory = session->getRequestFactory();
    
    auto valuemap = factory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketOpen);
    valuemap->setString(AccountID, kAccountId->s);
    valuemap->setString(OfferID, kOfferId->s);  // The ID of the instrument
    valuemap->setString(BuySell, kAmount->j > 0 ? O2G2::Buy : O2G2::Sell);
    valuemap->setInt(Amount, kAmount->j);
    valuemap->setString(CustomID, kCustomId->s); // The custom identifier of the order
    
    auto request = factory->createOrderRequest(valuemap);
    Q(!request, factory->getLastError());
    session->sendRequest(request);
    
    R 0;
}

extern "C" K createOCOOrder(K kAccountId, K kOfferId, K kAmount, K kBuyRate, K kSellRate, K kCustomId)
{
    Q(kAccountId->t != -11 || kOfferId->t != -7 || kAmount->t != -7 ||
      kBuyRate->t != -9 || kSellRate->t != -9 || kCustomId->t != -11, "type");
    
    auto factory = session->getRequestFactory();
    auto mainValueMap = factory->createValueMap();
    mainValueMap->setString(Command, kCustomId->s);
    
    auto child1 = factory->createValueMap();
    child1->setString(Command, kCustomId->s);
    child1->setString(OrderType, "SE");
    child1->setString(OfferID, kOfferId->s);
    child1->setDouble(Rate, kBuyRate->f);
    child1->setString(AccountID, kAccountId->s);
    child1->setString(BuySell, "B");
    child1->setInt(Amount, kAmount->j);
    mainValueMap->appendChild(child1);
    
    auto child2 = child1->clone();  // to reduce typing
    child2->setDouble(Rate, kSellRate->f);
    child2->setString(BuySell, "S");
    mainValueMap->appendChild(child2);
    
    auto orderRequest = factory->createOrderRequest(mainValueMap);
    Q(!orderRequest, factory->getLastError());
    session->sendRequest(orderRequest);
    
    R 0;
}

extern "C" K LoadLibrary(K x)
{
    O("\n");
    O("%s:\n",                          PROGRAM_NAME);
    O("release » %-5s\n",               BUILD_PROJECT_VERSION);
    O("os » %-5s\n",                    BUILD_OPERATING_SYSTEM);
    O("arch » %-5s\n",                  BUILD_PROCESSOR);
    O("git commit » %-5s\n",            BUILD_GIT_SHA1);
    O("git commit datetime » %-5s\n",   BUILD_GIT_DATE);
    O("kdb compatibility » %s.x\n",     BUILD_KX_VER);
    O("compiler flags » %-5s\n",        BUILD_COMPILER_FLAGS);
    O("\n");
    
    K keys = ktn(KS, 7);
    K vals = ktn(0, 7);
    
    kS(keys)[0] = ss((S) "connect");
    kS(keys)[1] = ss((S) "disconnect");
    kS(keys)[2] = ss((S) "isconnected");
    kS(keys)[3] = ss((S) "gethistprices");
    kS(keys)[4] = ss((S) "reqmktdata");
    kS(keys)[5] = ss((S) "createorder");
    kS(keys)[6] = ss((S) "createocoorder");
    
    kK(vals)[0] = dl((void *) connect, 4);
    kK(vals)[1] = dl((void *) disconnect, 1);
    kK(vals)[2] = dl((void *) isconnected, 1);
    kK(vals)[3] = dl((void *) gethistprices, 4);
    kK(vals)[4] = dl((void *) requestMarketData, 1);
    kK(vals)[5] = dl((void *) createOrder, 4);
    kK(vals)[6] = dl((void *) createOCOOrder, 6);
    
    R xD(keys, vals);
}


