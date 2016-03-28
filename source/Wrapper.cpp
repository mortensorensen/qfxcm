/*
 * Wrapper for FXCM's ForexConnect API
 *
 * Author: Morten Sorensen
 *
 */

#include "Wrapper.h"
#include "Helpers.h"
#define TO_CPP(w) (reinterpret_cast<Wrapper *>(w))
#define TO_C(w) (reinterpret_cast<wrapper *>(w))

#define DBG(x, e)                                                              \
{                                                                            \
if (x)                                                                     \
O("%s\n", e);                                                            \
}
#define Q(x, e) P(x, krr((S)e))

IO2GSession *session;
SessionStatusListener *sessionListener;
ResponseListener *responseListener;

// http://stackoverflow.com/questions/14917952/can-i-use-shared-library-created-in-c-in-a-c-program
//
// K Wrapper::connect(K host, K user, K password)
// {
//    session = CO2GTransport::createSession();
//    sessionListener = new SessionStatusListener(session, false, "", "");
//    session->subscribeSessionStatus(sessionListener);
//
////    return (K)login(session, sessionListener, "");
//    return 0;
// }
//
// wrapper *wrapper_create() {
//    wrapper *w = TO_C(new Wrapper);
//    return w;
// }
//
// wrapper *wrapper_create_init()
// {
//    wrapper *w = TO_C(new Wrapper);
//    return w;
// }
//
// void wrapper_destroy(wrapper *w)
// {
//    delete TO_CPP(w);
// }

void accumulatePrices(IO2GResponse *response) {
    if (response != 0) {
        if (response->getType() == MarketDataSnapshot) {
            O("RequestID='%s' completed\n", response->getRequestID());
            
            O2G2Ptr<IO2GResponseReaderFactory> factory = session->getResponseReaderFactory();
            
            if (factory) {
                O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader =
                    factory->createMarketDataSnapshotReader(response);
                
                if (reader) {
                    J n = reader->size();
                    
                    if (reader->isBar()) {
                        K cols = ktn(KS, 10);
                        kS(cols)[0] = ss((S) "DateTime");
                        kS(cols)[1] = ss((S) "BidOpen");
                        kS(cols)[2] = ss((S) "BidHigh");
                        kS(cols)[3] = ss((S) "BidLow");
                        kS(cols)[4] = ss((S) "BidClose");
                        kS(cols)[5] = ss((S) "AskOpen");
                        kS(cols)[6] = ss((S) "AskHigh");
                        kS(cols)[7] = ss((S) "AskLow");
                        kS(cols)[8] = ss((S) "AskClose");
                        kS(cols)[9] = ss((S) "Volume");
                        
                        K dateTime = ktn(KZ, n);
                        K bidOpen = ktn(KF, n);
                        K bidHigh = ktn(KF, n);
                        K bidLow = ktn(KF, n);
                        K bidClose = ktn(KF, n);
                        K askOpen = ktn(KF, n);
                        K askHigh = ktn(KF, n);
                        K askLow = ktn(KF, n);
                        K askClose = ktn(KF, n);
                        K volume = ktn(KJ, n);
                        
                        DO(n, kF(dateTime)[i] = zo(reader->getDate(i));
                           kF(bidOpen)[i] = reader->getBidOpen(i);
                           kF(bidHigh)[i] = reader->getBidHigh(i);
                           kF(bidLow)[i] = reader->getBidLow(i);
                           kF(bidClose)[i] = reader->getBidClose(i);
                           kF(askOpen)[i] = reader->getAskOpen(i);
                           kF(askHigh)[i] = reader->getAskHigh(i);
                           kF(askLow)[i] = reader->getAskLow(i);
                           kF(askClose)[i] = reader->getAskClose(i);
                           kF(volume)[i] = reader->getVolume(i);)
                        
                        K table = xT(xD(cols, knk(10, dateTime, bidOpen, bidHigh, bidLow, bidClose,
                                                  askOpen, askHigh, askLow, askClose, volume)));
                        consume_event("onhistprice", table);
                    } else {
                        K cols = ktn(KS, 3);
                        kS(cols)[0] = ss((S) "DateTime");
                        kS(cols)[1] = ss((S) "Bid");
                        kS(cols)[2] = ss((S) "Ask");
                        
                        K dateTime = ktn(KF, n);
                        K bid = ktn(KF, n);
                        K ask = ktn(KF, n);
                        
                        DO(n,
                           kF(dateTime)[i] = zo(reader->getDate(i));
                           kF(bid)[i] = reader->getBid(i);
                           kF(ask)[i] = reader->getAsk(i);)
                        
                        K table = xT(xD(cols, knk(3, dateTime, bid, ask)));
                        consume_event("onhistprice", table);
                    }
                }
            }
        }
    }
}

extern "C" K connect(K host, K user, K password, K connection) {
    Q(sessionListener && sessionListener->isConnected(), "session");
    Q(strcasecmp(connection->s, "Real") != 0 &&
      strcasecmp(connection->s, "Demo") != 0,
      "connection");
    
    session = CO2GTransport::createSession();
    sessionListener = new SessionStatusListener(session, false, "", "");
    session->subscribeSessionStatus(sessionListener);
    
    sessionListener->reset();
    session->login(user->s, password->s, host->s, connection->s);
    sessionListener->waitEvents();
    
    if (sessionListener->isConnected()) {
        responseListener = new ResponseListener(session);
        session->subscribeResponse(responseListener);
    }
    
    R kb(sessionListener->isConnected());
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
    
//    delete session;
    R 0;
}

extern "C" K isconnected(K ignore) {
    R kb(sessionListener && sessionListener->isConnected());
}

extern "C" K gethistprices(K kInstrument, K kTimeframe, K kDtFrom, K kDtTo) {
    Q(!session || !sessionListener, "session");
    
    O2G2Ptr<IO2GRequestFactory> factory = session->getRequestFactory();
    Q(!factory, "factory");
    
    // Find timeframe by identifer
    O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = factory->getTimeFrameCollection();
    O2G2Ptr<IO2GTimeframe> timeframe = timeframeCollection->get(kTimeframe->s);
    Q(!timeframe, "timeframe");
    
    O2G2Ptr<IO2GRequest> request =
        factory->createMarketDataSnapshotRequestInstrument(kInstrument->s, timeframe, timeframe->getQueryDepth());
    DATE dtFirst, dtFrom;
    KTimeToOleTime(kDtTo, &dtFirst);
    KTimeToOleTime(kDtFrom, &dtFrom);
    
    // There is a limit for retured candles amount
    do {
        factory->fillMarketDataSnapshotRequestTime(request, dtFrom, dtFirst, false);
        responseListener->setRequestID(request->getRequestID());
        session->sendRequest(request);
        Q(!responseListener->waitEvents(), "timeout");
        
        // shift "to" bound to oldest datetime of returned data
        O2G2Ptr<IO2GResponse> response = responseListener->getResponse();
        
        if (response && (response->getType() == MarketDataSnapshot)) {
            O2G2Ptr<IO2GResponseReaderFactory> readerFactory = session->getResponseReaderFactory();
            
            if (readerFactory) {
                O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader =
                    readerFactory->createMarketDataSnapshotReader(response);
                
                if (reader->size() > 0) {
                    if (std::abs(dtFirst - reader->getDate(0)) > 0.0001)
                        dtFirst = reader->getDate(0); // earliest datetime of returned data
                    else
                        break;
                } else {
                    O("0 rows received\n");
                    break;
                }
            }
            accumulatePrices(response);
        } else {
            break;
        }
    } while (dtFirst - dtFrom > 0.0001);
    
    R kb(1);
}
