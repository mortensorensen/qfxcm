#ifndef QFOREXCONNECT_H
#define QFOREXCONNECT_H

#include <config.h>

#include "stdafx.h"
#include "SessionStatusListener.h"
#include "ResponseListener.h"
#include "TableListener.h"

struct LoginParams
{
    std::string mLogin;
    std::string mPassword;
    std::string mConnection;
    std::string mUrl;
  
    void set(const std::string &login,
             const std::string &password,
             const std::string &connection,
             const std::string &url);
    bool areSet();
};

extern "C"
{
    K receiveData(I x);
    K version(K x);
    K LoadLibrary(K x);
    K setLoginParams(K login, K password, K connection, K url);
    K login(K x);
    K logout(K x);
    K isConnected(K x);
    K getAccountId(K x);
    K getUsedMargin(K x);
    K getBalance(K x);
    K getOffers(K x);
    K getTrades(K x);
    K getBid(K instrument);
    K getAsk(K instrument);
    K sendMessage(K dict);
    K getHistoricalPrices(K instrument, K begin, K end, K timeframe);
    K subscribeOffers(K instrument);
    K unsubscribeOffers(K x);
    K getServerTime(K x);
    K getBaseUnitSize(K x);
}

static std::map<std::string, std::string> getOffersMap();
static IO2GAccountTableRow* getAccount();
static IO2GTableManager* getLoadedTableManager();
static K getPricesFromResponse(IO2GResponse *response);
static IO2GValueMap* convertToValueMap(K dict, const char *&error);

#endif