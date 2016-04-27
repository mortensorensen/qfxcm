#include "ForexConnectClient.h"
#include <string.h>
#include "Helpers.h"

using namespace ForexConnect;

ForexConnectClient::ForexConnectClient()
    : mpSession(nullptr),
    mpListener(nullptr),
    mpResponseListener(nullptr),
    mpLoginRules(nullptr),
    mpAccountRow(nullptr),
    mpResponseReaderFactory(nullptr),
    mpRequestFactory(nullptr),
    mpTableListener(new TableListener())
{
    init();
}

ForexConnectClient::ForexConnectClient(const std::string& login,
                                       const std::string& password,
                                       const std::string& connection,
                                       const std::string& url)
{
}

ForexConnectClient::~ForexConnectClient()
{
    if (mpRequestFactory) mpRequestFactory->release();
    if (mpAccountRow) mpAccountRow->release();
    if (mpLoginRules) mpLoginRules->release();
    if (mpResponseReaderFactory) mpResponseReaderFactory->release();
    if (mpResponseListener) mpResponseListener->release();
    if (mIsConnected) {
        logout();
    }
    mpSession->unsubscribeSessionStatus(mpListener);
    mpListener->release();
    mpSession->release();
}

void ForexConnectClient::init()
{
    mpSession = CO2GTransport::createSession();
    mpListener = new SessionStatusListener(mpSession, false, "", "");
    mpSession->subscribeSessionStatus(mpListener);
    mpSession->useTableManager(Yes, 0);
    
    mpResponseListener = new ResponseListener(mpSession);
    mpSession->subscribeResponse(mpResponseListener);

}

K ForexConnectClient::login(K& login, K& password, K& connection, K& url)
{
    Q(login->t != -KS || password->t != -KS || connection->t != -KS || url->t != -KS, "type");
    Q(mIsConnected, "already connected");
    Q(strcasecmp(connection->s, "Real") != 0 &&
      strcasecmp(connection->s, "Demo") != 0,
      "connection");
    
    mpListener->reset();
    mpSession->login(login->s, password->s, url->s, connection->s);
    mIsConnected = mpListener->waitEvents() && mpListener->isConnected();
    Q(!mIsConnected, "Couldn't connect");
    
    mpLoginRules = mpSession->getLoginRules();
    if (!mpLoginRules->isTableLoadedByDefault(Accounts)) {
        logout();
        R krr((S) "Accounts table not loaded");
    }

    auto response = mpLoginRules->getTableRefreshResponse(Accounts);
    if (!response) {
        logout();
        R krr((S) "No response to refresh accounts table request");
    }

    mpResponseReaderFactory = mpSession->getResponseReaderFactory();
    auto accountsResponseReader = mpResponseReaderFactory->createAccountsTableReader(response);
    mpAccountRow = accountsResponseReader->getRow(0);
    mAccountID = mpAccountRow->getAccountID();
    
    mpResponseListener = new ResponseListener(mpSession);
    mpSession->subscribeResponse(mpResponseListener);
    
    mpRequestFactory = mpSession->getRequestFactory();
    
    R 0;
}

void ForexConnectClient::logout()
{
    mpListener->reset();
    if (!mIsConnected) {
        O("already logged out");
        return;
    }
    mpSession->logout();
    mpListener->waitEvents();
    mIsConnected = false;
}

K ForexConnectClient::isConnected() const
{
    R kb(mIsConnected);
}

K ForexConnectClient::getAccountID() const
{
    Q(!mIsConnected, "connection");
    R ks((S)mpAccountRow->getAccountID());
}

K ForexConnectClient::getUsedMargin() const
{
    Q(!mIsConnected, "connection");
    R kf(mpAccountRow->getUsedMargin());
}

K ForexConnectClient::getBalance()
{
    Q(!mIsConnected, "connection");
    auto account = getAccount();
    R kf((!account) ? mpAccountRow->getBalance() : account->getBalance());
}

std::map<std::string, std::string> ForexConnectClient::getOffers()
{
    std::map<std::string, std::string> offers;
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = nullptr;
    IO2GTableIterator it;
    while (offersTable->getNextRow(it, offerRow)) {
        offers[offerRow->getInstrument()] = offerRow->getOfferID();
        offerRow->release();
    }
    return offers;
}

K ForexConnectClient::getBid(K& instrument)
{
    Q(instrument->t != -KS, "type");
    Q(!mIsConnected, "connection");
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = nullptr;
    IO2GTableIterator iter;
    while (offersTable->getNextRow(iter, offerRow)) {
        if (strcasecmp(offerRow->getInstrument(), instrument->s) == 0) {
            K bid = kf(offerRow->getBid());
            offerRow->release();
            R bid;
        }
        offerRow->release();
    }
    R krr((S) "instrument");
}

K ForexConnectClient::getAsk(K& instrument)
{
    Q(instrument->t != -KS, "type");
    Q(!mIsConnected, "connection");
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = nullptr;
    IO2GTableIterator iter;
    while (offersTable->getNextRow(iter, offerRow)) {
        if (strcasecmp(offerRow->getInstrument(), instrument->s) == 0) {
            K ask = kf(offerRow->getAsk());
            offerRow->release();
            R ask;
        }
        offerRow->release();
    }
    R krr((S) "instrument");
}

K ForexConnectClient::getTrades()
{
    Q(!mIsConnected, "connection");
    auto tableManager = getLoadedTableManager();
    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
    IO2GTradeTableRow* tradeRow = nullptr;
    IO2GTableIterator tableIterator;
    auto offers = getOffers();
    
    K headers = ktn(KS, 6);
    kS(headers)[0] = ss((S)"tradeid");
    kS(headers)[1] = ss((S)"instrument");
    kS(headers)[2] = ss((S)"openrate");
    kS(headers)[3] = ss((S)"amount");
    kS(headers)[4] = ss((S)"opentime");
    kS(headers)[5] = ss((S)"grosspnl");
    
    K tradeId    = ktn(KS, 0);
    K instrument = ktn(KS, 0);
    K openRate   = ktn(KF, 0);
    K amount     = ktn(KJ, 0);
    K openTime   = ktn(KZ, 0);
    K grossPnL   = ktn(KF, 0);
    
    while (tradesTable->getNextRow(tableIterator, tradeRow)) {
        // TODO: FIX
        auto it = std::find_if(offers.begin(), offers.end(),
                               [tradeRow](const std::pair<std::string, std::string>& x) -> bool {
                                   return x.second == tradeRow->getOfferID();
                               });
        
//        Q(it == offers.end(), "Could not get offer table row");
        
        js(&tradeId,    ss((S)tradeRow->getTradeID()));
        js(&instrument, ss((S)it->first.c_str()));
        ja(&openRate,   kf(tradeRow->getOpenRate()));
        ja(&amount,     kj(tradeRow->getAmount() > 0 ?
                           tradeRow->getAmount() :
                           -tradeRow->getAmount()));
        ja(&openTime,   toKTime(tradeRow->getOpenTime()));
        ja(&grossPnL,   kf(tradeRow->getGrossPL()));
        
        tradeRow->release();
    }
    
    R xT(xD(headers, knk(6, tradeId, instrument, openRate, amount, openTime, grossPnL)));
}

K ForexConnectClient::openPosition(K& dict)
{
    Q(!mIsConnected, "connection");
//    Q(instrument->t != -KS || amount->t != -KJ, "type");
//    if (amount->j == 0) R 0;
    
    // Find offer ID
//    auto offers = getOffers();
//    auto offerItr = offers.find(instrument->s);
//    Q(offerItr == offers.end(), "instrument");
//    std::string offerID = offerItr->second;
    
//    auto tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
//    uint iBaseUnitSize = tradingSettingsProvider->getBaseUnitSize(instrument->s, mpAccountRow);
    
    auto valuemap = createValueMap(dict);
    Q(!valuemap, "error");
    auto request = mpRequestFactory->createOrderRequest(valuemap);
    if (!request) {
        R krr((S)mpRequestFactory->getLastError());
    }
    mpResponseListener->setRequestID(request->getRequestID());
    mpSession->sendRequest(request);
    if (mpResponseListener->waitEvents()) {
        Sleep(1000); // Wait for the balance update
        O("Done!\n");
        R kb(true);
    }
    R krr((S) "Response waiting timeout expired");
}

K ForexConnectClient::closePosition(K& dict)
{
    Q(!mIsConnected, "connection");
//    auto tableManager = getLoadedTableManager();
//    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
//    IO2GTradeTableRow *tradeRow = nullptr;
//    IO2GTableIterator tableIterator;
//    while (tradesTable->getNextRow(tableIterator, tradeRow)) {
//        if (strcmp(tradeID->s, tradeRow->getTradeID()) == 0)
//            break;
//    }
//    if (!tradeRow) {
//        R krr((S)string_format("Could not found trade with ID = %s", tradeID->s).c_str());
//    }
    auto valuemap = createValueMap(dict);
    auto request = mpRequestFactory->createOrderRequest(valuemap);
    if (!request) {
        R krr((S) mpRequestFactory->getLastError());
    }
    mpResponseListener->setRequestID(request->getRequestID());
    mpSession->sendRequest(request);
    if (mpResponseListener->waitEvents()) {
        Sleep(1000); // Wait for the balance update
        O("Done!\n");
        R kb(true);
    }
    R krr((S) "Response waiting timeout expired");
}

K ForexConnectClient::getHistoricalPrices(K instrument, K from, K to, K timeFrame)
{
    Q(!(instrument->t == -KS &&
        (from->t == -KZ || from->t == -KP) &&
        (to->t == -KZ || to->t == -KP) &&
        timeFrame->t == -KS), "type");
    Q(!mIsConnected, "connection");
    O2G2Ptr<IO2GTimeframeCollection> timeframeCollection = mpRequestFactory->getTimeFrameCollection();
    O2G2Ptr<IO2GTimeframe> timeframe = timeframeCollection->get(timeFrame->s);
    Q(!timeframe, "Timeframe is incorrect");
    
    O2G2Ptr<IO2GRequest> request = mpRequestFactory->createMarketDataSnapshotRequestInstrument(instrument->s, timeframe, timeframe->getQueryDepth());
    K prices = knk(0);
    DATE dtFrom = toOleTime(from);
    DATE dtFirst = toOleTime(to);
    do
    {
        mpRequestFactory->fillMarketDataSnapshotRequestTime(request, dtFrom, dtFirst, false);
        mpResponseListener->setRequestID(request->getRequestID());
        mpSession->sendRequest(request);
        Q(!mpResponseListener->waitEvents(), "Response waiting timeout expired");
        
        // shift "to" bound to oldest datetime of returned data
        O2G2Ptr<IO2GResponse> response = mpResponseListener->getResponse();
        if (response && response->getType() == MarketDataSnapshot) {
            O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
            Q(reader->size() == 0, "0 rows received");
            if (fabs(dtFirst - reader->getDate(0)) <= 0.0001)
                break;
            
            dtFirst = reader->getDate(0); // earliest datetime of return data
            jv(&prices, getPricesFromResponse(response));
        } else {
            break;
        }
    } while (dtFirst - dtFrom > 0.0001);

    K headers = ktn(KS, 10);
    kS(headers)[0] = ss((S)"date");
    kS(headers)[1] = ss((S)"bidopen");
    kS(headers)[2] = ss((S)"bidhigh");
    kS(headers)[3] = ss((S)"bidlow");
    kS(headers)[4] = ss((S)"bidclose");
    kS(headers)[5] = ss((S)"askopen");
    kS(headers)[6] = ss((S)"askhigh");
    kS(headers)[7] = ss((S)"asklow");
    kS(headers)[8] = ss((S)"askclose");
    kS(headers)[9] = ss((S)"volume");
    
    // TODO: Implement for ticks
    
    R xT(xD(headers, prices));
}

K ForexConnectClient::subscribeOffers(K& instrument)
{
    Q(instrument->t != -KS, "type");
    Q(!mIsConnected, "connection");
    
    // Print current quotes
//    auto tableManager = getLoadedTableManager();
//    auto offers = (IO2GOffersTable *)tableManager->getTable(Offers);
//    mpTableListener->printOffers(offers, "");
//    
//    mpTableListener->setInstrument(instrument->s);
//    mpTableListener->subscribeEvents(tableManager);
    
    
    O2G2Ptr<IO2GRequest> offersRequest = mpRequestFactory->createRefreshTableRequest(Offers);
    mpResponseListener->setRequestID(offersRequest->getRequestID());
    mpSession->sendRequest(offersRequest);
    
    R 0;
}

K ForexConnectClient::unsubscribeOffers(K x)
{
    mpTableListener->unsubscribeEvents(getLoadedTableManager());
    R 0;
}

K ForexConnectClient::getServerTime(K x)
{
    Q(!mIsConnected, "connection");
    R toKTime(mpSession->getServerTime());
}

K ForexConnectClient::getBaseUnitSize(K instrument)
{
    Q(instrument->t != -KS, "type");
    Q(!mIsConnected, "connection");
    O2G2Ptr<IO2GTradingSettingsProvider> tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
    R kj(tradingSettingsProvider->getBaseUnitSize(instrument->s, mpAccountRow));
}

IO2GAccountTableRow* ForexConnectClient::getAccount()
{
    auto tableManager = getLoadedTableManager();
    auto accountsTable = static_cast<IO2GAccountsTable*>(tableManager->getTable(Accounts));
    
    IO2GAccountTableRow *account = NULL;
    IO2GTableIterator it;
    while (accountsTable->getNextRow(it, account)) {
        if (mAccountID.size() == 0 || strcmp(account->getAccountID(), mAccountID.c_str()) == 0) {
            if (strcmp(account->getMarginCallFlag(), "N") == 0 &&
                (strcmp(account->getAccountKind(), "32") == 0 ||
                 strcmp(account->getAccountKind(), "36") == 0))
            {
                return account;
            }
        }
        account->release();
    }
    return nullptr;
}

IO2GTableManager* ForexConnectClient::getLoadedTableManager()
{
    O2G2Ptr<IO2GTableManager> tableManager = mpSession->getTableManager();
    auto managerStatus = tableManager->getStatus();
    while (managerStatus == TablesLoading) {
        Sleep(50);
        managerStatus = tableManager->getStatus();
    }
    
    if (managerStatus == TablesLoadFailed) {
        O("Cannot refresh all tables of table manager\n");
        return nullptr;
    }
    return tableManager.Detach();
}

K ForexConnectClient::getPricesFromResponse(IO2GResponse *response)
{
    if (!response || response->getType() != MarketDataSnapshot) R 0;
    
    O("Request with RequestID='%s' is completed:\n", response->getRequestID());
    
    O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
    if (!reader) R 0;
    
    size_t nPrices = reader->size() - 1;
    
    if (reader->isBar()) {
        K date = ktn(KZ, nPrices);
        K bidopen = ktn(KF, nPrices);
        K bidhigh = ktn(KF, nPrices);
        K bidlow  = ktn(KF, nPrices);
        K bidclose = ktn(KF, nPrices);
        K askopen = ktn(KF, nPrices);
        K askhigh = ktn(KF, nPrices);
        K asklow  = ktn(KF, nPrices);
        K askclose = ktn(KF, nPrices);
        K volume = ktn(KJ, nPrices);
        
        DO(nPrices,
           kF(date)[i] = toKTime(reader->getDate(nPrices - i))->f;
           kF(bidopen)[i] = reader->getBidOpen(nPrices - i);
           kF(bidhigh)[i] = reader->getBidHigh(nPrices - i);
           kF(bidlow)[i] = reader->getBidLow(nPrices - i);
           kF(bidclose)[i] = reader->getBidClose(nPrices - i);
           kF(askopen)[i] = reader->getAskOpen(nPrices - i);
           kF(askhigh)[i] = reader->getAskHigh(nPrices - i);
           kF(asklow)[i] = reader->getAskLow(nPrices - i);
           kF(askclose)[i] = reader->getAskClose(nPrices - i);
           kJ(volume)[i] = reader->getVolume(nPrices - i));
        
        R knk(10, date, bidopen, bidhigh, bidlow, bidclose, askopen, askhigh, asklow, askclose, volume);
    } else {
        K date = ktn(KZ, nPrices);
        K bid  = ktn(KF, nPrices);
        K ask  = ktn(KF, nPrices);
        
        DO(nPrices,
           kF(date)[i] = toKTime(reader->getDate(nPrices - i))->f;
           kF(bid)[i] = reader->getBidOpen(nPrices - i);
           kF(ask)[i] = reader->getAskOpen(nPrices - i));
        
        R knk(3, date, bid, ask);
    }
}

IO2GValueMap* ForexConnectClient::createValueMap(K &dict)
{
    if (dict->t != 99) {
        O("createValueMap only accepts dicts");
        return nullptr;
    }
    
    auto valuemap = mpRequestFactory->createValueMap();
    auto keys = kK(dict)[0];
    auto vals = kK(dict)[1];
    O2GRequestParamsEnum key;
    int type;
    K val;
    bool hasError = false;
    
    for (int i = 0; i < keys->n; i++) {
        key = static_cast<O2GRequestParamsEnum>(kJ(keys)[i]);
        val = kK(vals)[i];
        type = val->t;
        if (type == -KS || type == -KC) {
            valuemap->setString(key, val->s);
        } else if (type == -KI || type == -KJ) {
            valuemap->setInt(key, val->j);
        } else if (type == -KF) {
            valuemap->setDouble(key, val->f);
        } else if (type == -KB) {
            valuemap->setBoolean(key, val->g);
        } else {
            O("Type %i not found for key %i\n", val->t, (int)key);
            hasError = true;
            break;
        }
    }
    return hasError ? nullptr : valuemap;
}