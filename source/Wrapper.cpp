/*
 * Wrapper for FXCM's ForexConnect API
 *
 * Author: Morten Sorensen
 *
 */

#include "Wrapper.h"
#include "Helpers.h"

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

static bool KTimeToOleTime(K t, F *mDateTo) {
    struct tm tmBuf = *lt(t->i);
    CO2GDateUtils::CTimeToOleTime(&tmBuf, mDateTo);
    R kb(1);
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
    
    auto accumulator = std::make_unique<MarketDataAccumulator>();

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
            
            O("RequestID='%s' completed\n", response->getRequestID());
            
            if (readerFactory) {
                O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader =
                    readerFactory->createMarketDataSnapshotReader(response);
            
                accumulator->addReader(reader);
                
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
        } else {
            break;
        }
    } while (dtFirst - dtFrom > 0.0001);
    
    R accumulator->getTable();
}
