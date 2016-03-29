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
    Q(host->t != -11 || user->t != -11 || password->t != -11 || connection->t != -11, "type");
    Q(sessionListener && sessionListener->isConnected(), "session");
    Q(strcasecmp(connection->s, "Real") != 0 &&
      strcasecmp(connection->s, "Demo") != 0,
      "connection");
    
    session = CO2GTransport::createSession();
    sessionListener = new SessionStatusListener(session, false, "", "");
    session->subscribeSessionStatus(sessionListener);
    
    sessionListener->reset();
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
    
//    delete session;
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
    
    R accumulator->getTable();
}
