#ifndef FOREX_CONNECT_CLIENT_H
#define FOREX_CONNECT_CLIENT_H

#include <config.h>

#include "stdafx.h"
#include "SessionStatusListener.h"
#include "ResponseListener.h"
#include "TableListener.h"

namespace ForexConnect {
    class ForexConnectClient
    {
    public:
        ForexConnectClient();
        ForexConnectClient(const std::string& login,
                           const std::string& password,
                           const std::string& connection,
                           const std::string& url = DEFAULT_FXCM_URL);
        ~ForexConnectClient();
        
        K login(K& login, K& password, K& connection, K& url);
        void logout();
        K isConnected() const;
        K getAccountID() const;
        K getUsedMargin() const;
        K getBalance();
        std::map<std::string, std::string> getOffers();
        K getBid(K& instrument);
        K getAsk(K& instrument);
        K getTrades();
        K openPosition(K& dict);
        K closePosition(K& tradeID);
        K getHistoricalPrices(K instrument, K from, K to, K timeFrame);
        K subscribeOffers(K& instrument);
        K unsubscribeOffers(K x);
        K getServerTime(K x);
        K getBaseUnitSize(K instrument);
        
    private:
        IO2GSession* mpSession;
        SessionStatusListener* mpListener;
        ResponseListener* mpResponseListener;
        IO2GLoginRules* mpLoginRules;
        IO2GAccountRow* mpAccountRow;
        IO2GResponseReaderFactory* mpResponseReaderFactory;
        IO2GRequestFactory* mpRequestFactory;
        std::string mAccountID;
        bool mIsConnected;
        TableListener* mpTableListener;
        
        void init();
        IO2GAccountTableRow* getAccount();
        IO2GTableManager* getLoadedTableManager();
        K getPricesFromResponse(IO2GResponse* response);
        IO2GValueMap *createValueMap(K &dict);
        
        static bool findOfferRowBySymbol(IO2GOfferRow *row, std::string symbol)
        {
            return (symbol == row->getInstrument() && row->getSubscriptionStatus()[0] == 'T');
        }
        
        static bool findOfferRowByOfferId(IO2GOfferRow *row, std::string offerId)
        {
            return (offerId == row->getOfferID());
        }
        
        static IO2GOffersTableResponseReader* getOffersReader(IO2GResponseReaderFactory* readerFactory,
                                                              IO2GResponse* response)
        {
            return readerFactory->createOffersTableReader(response);
        }
        
        static IO2GTradesTableResponseReader* getTradesReader(IO2GResponseReaderFactory* readerFactory,
                                                              IO2GResponse* response)
        {
            return readerFactory->createTradesTableReader(response);
        }
    };
}

#endif