#ifndef FOREX_CONNECT_CLIENT_H
#define FOREX_CONNECT_CLIENT_H

#include <config.h>

#include "stdafx.h"
#include "SessionStatusListener.h"
#include "ResponseListener.h"
#include "Offer.h"

namespace ForexConnect {
    struct LoginParams
    {
        std::string mLogin;
        std::string mPassword;
        std::string mConnection;
        std::string mUrl;
        
        LoginParams();
        LoginParams(const std::string& login,
                    const std::string& password,
                    const std::string& connection,
                    const std::string& url = DEFAULT_FXCM_URL);
        
        bool areSet();
    };
    std::ostream& operator<<(std::ostream& out, LoginParams const& lp);
    
    struct TradeInfo
    {
        std::string mInstrument;
        std::string mTradeID;
        std::string mBuySell;
        double mOpenRate;
        int mAmount;
        DATE mOpenDate;
        double mGrossPL;
        bool operator==(const TradeInfo& other);
        bool operator!=(const TradeInfo& other);
        TradeInfo();
    };
    std::ostream& operator<<(std::ostream& out, TradeInfo const& ti);
    
    struct Prices
    {
        DATE mDate;
        double mOpen;
        double mHigh;
        double mLow;
        double mClose;
        bool operator==(const Prices& other);
        bool operator!=(const Prices& other);
        Prices();
        Prices(DATE date,
               double value);
        Prices(DATE date,
               double open,
               double high,
               double low,
               double close);
    };
    std::ostream& operator<<(std::ostream& out, Prices const& pr);
    
    class ForexConnectClient
    {
    public:
        ForexConnectClient();
        ForexConnectClient(const LoginParams& loginParams);
        ForexConnectClient(const std::string& login,
                           const std::string& password,
                           const std::string& connection,
                           const std::string& url = DEFAULT_FXCM_URL);
        ~ForexConnectClient();
        
        bool login();
        bool login(const std::string& login,
                   const std::string& password,
                   const std::string& connection,
                   const std::string& url = DEFAULT_FXCM_URL);
        void logout();
        bool isConnected() const;
        std::string getAccountID() const;
        double getUsedMargin() const;
        double getBalance();
        std::map<std::string, std::string> getOffers();
        double getBid(const std::string& instrument);
        double getAsk(const std::string& instrument);
        std::vector<TradeInfo> getTrades();
        bool openPosition(const std::string& instrument,
                          const std::string& buysell,
                          int amount);
        bool closePosition(const std::string& tradeID);
        std::vector<Prices> getHistoricalPrices(const std::string& instrument,
                                                const DATE from,
                                                const DATE to,
                                                const std::string& timeFrame = std::string("m1"));
        
    private:
        void init();
        IO2GAccountTableRow* getAccount();
        IO2GTableManager* getLoadedTableManager();
        std::vector<Prices> getPricesFromResponse(IO2GResponse* response);
        LoginParams mLoginParams;
        IO2GSession* mpSession;
        SessionStatusListener* mpListener;
        ResponseListener* mpResponseListener;
        IO2GLoginRules* mpLoginRules;
        IO2GAccountRow* mpAccountRow;
        IO2GResponseReaderFactory* mpResponseReaderFactory;
        IO2GRequestFactory* mpRequestFactory;
        std::string mAccountID;
        bool mIsConnected;
        
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
    
    void setLogLevel(int level);
}

#endif