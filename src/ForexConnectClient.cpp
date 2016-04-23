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
    
    K headers = vector_to_k_list(std::vector<std::string> {
        "instrument", "tradeid", "openrate", "amount", "opentime", "grosspnl"
    });

    K instrument = ktn(KS, 0);
    K tradeId    = ktn(KS, 0);
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
        
        js(&instrument, ss((S)it->first.c_str()));
        js(&tradeId,    ss((S)tradeRow->getTradeID()));
        jk(&openRate,   kf(tradeRow->getOpenRate()));
        jk(&amount,     kj(tradeRow->getAmount() > 0 ?
                           tradeRow->getAmount() :
                           -tradeRow->getAmount()));
        jk(&openTime,   kz(toKTime(tradeRow->getOpenTime())));
        jk(&grossPnL,   kf(tradeRow->getGrossPL()));
        
        tradeRow->release();
    }
    
    R xT(xD(headers, knk(6, instrument, tradeId, openRate, amount, openTime, grossPnL)));
}

K ForexConnectClient::openPosition(K& instrument, K& amount)
{
    Q(!mIsConnected, "connection");
    Q(instrument->t != -KS || amount->t != -KJ, "type");
    if (amount->j == 0) R 0;
    
    // Find offer ID
    auto offers = getOffers();
    std::string offerID;
    auto offer_itr = offers.find(instrument->s);
    if (offer_itr != offers.end()) {
        offerID = offer_itr->second;
    } else {
        R krr((S) "instrument");
    }
    
    auto tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
    uint iBaseUnitSize = tradingSettingsProvider->getBaseUnitSize(instrument->s, mpAccountRow);
    auto valuemap = mpRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketOpen);
    valuemap->setString(AccountID, mAccountID.c_str());
    valuemap->setString(OfferID, offerID.c_str());
    valuemap->setString(BuySell, amount->j > 0 ? O2G2::Buy : O2G2::Sell);
    valuemap->setInt(Amount, amount->j * iBaseUnitSize);
    valuemap->setString(TimeInForce, O2G2::TIF::IOC);
    valuemap->setString(CustomID, "TrueMarketOrder");
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

K ForexConnectClient::closePosition(K& tradeID)
{
    Q(!mIsConnected, "connection");
    auto tableManager = getLoadedTableManager();
    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
    IO2GTradeTableRow *tradeRow = nullptr;
    IO2GTableIterator tableIterator;
    while (tradesTable->getNextRow(tableIterator, tradeRow)) {
        if (strcmp(tradeID->s, tradeRow->getTradeID()) == 0)
            break;
    }
    if (!tradeRow) {
        R krr((S)string_format("Could not found trade with ID = %s", tradeID->s).c_str());
    }
    auto valuemap = mpRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketClose);
    valuemap->setString(AccountID, mAccountID.c_str());
    valuemap->setString(OfferID, tradeRow->getOfferID());
    valuemap->setString(TradeID, tradeID->s);
    valuemap->setString(BuySell, (strcmp(tradeRow->getBuySell(), O2G2::Buy) == 0) ? O2G2::Sell : O2G2::Buy);
    tradeRow->release();
    valuemap->setInt(Amount, tradeRow->getAmount());
    valuemap->setString(CustomID, "CloseMarketOrder");
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

K ForexConnectClient::getHistoricalPrices(const std::string &instrument,
                                                            const DATE from,
                                                            const DATE to,
                                                            const std::string& timeFrame)
{
    K prices;
    auto timeframeCollection = mpRequestFactory->getTimeFrameCollection();
    auto timeframe = timeframeCollection->get(timeFrame.c_str());
    Q(!timeframe, "Timeframe is incorrect");
    
    auto request = mpRequestFactory->createMarketDataSnapshotRequestInstrument(instrument.c_str(), timeframe, timeframe->getQueryDepth());
    DATE first = to;
    do
    {
        mpRequestFactory->fillMarketDataSnapshotRequestTime(request, from, first, false);
        mpResponseListener->setRequestID(request->getRequestID());
        mpSession->sendRequest(request);
        Q(!mpResponseListener->waitEvents(), "Response waiting timeout expired");
        
        // shift "to" bound to oldest datetime of returned data
        auto response = mpResponseListener->getResponse();
        if (response && response->getType() == MarketDataSnapshot) {
            auto reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
            Q(reader->size() == 0, "0 rows received");
            if (fabs(first - reader->getDate(0)) <= 0.0001)
                break;
            
            first = reader->getDate(0); // earliest datetime of return data
            jk(&prices, getPricesFromResponse(response));
        } else {
            break;
        }
    } while (first - from > 0.0001);

    R prices;
}

K ForexConnectClient::subscribeOffers(K& instrument)
{
    Q(instrument->t != -KS, "type");
    Q(!mIsConnected, "connection");
    
    // Print current quotes
    auto tableManager = getLoadedTableManager();
    auto offers = (IO2GOffersTable *)tableManager->getTable(Offers);
    mpTableListener->printOffers(offers, "");
    
    mpTableListener->setInstrument(instrument->s);
    mpTableListener->subscribeEvents(tableManager);
    
    R 0;
}

K ForexConnectClient::unsubscribeOffers(K x)
{
    mpTableListener->unsubscribeEvents(getLoadedTableManager());
    R 0;
}

K ForexConnectClient::getservertime(K x)
{
    Q(!mIsConnected, "connection");
    R kz(toKTime(mpSession->getServerTime()));
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
    
    auto reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
    if (!reader) R 0;
    
    size_t nPrices = reader->size() - 1;
    
    if (reader->isBar()) {
        K headers = vector_to_k_list(std::vector<std::string> {"date", "askopen", "askhigh", "asklow", "askclose"});
        K date = ktn(KZ, nPrices);
        K open = ktn(KF, nPrices);
        K high = ktn(KF, nPrices);
        K low  = ktn(KF, nPrices);
        K close = ktn(KF, nPrices);
        K volume = ktn(KJ, nPrices);
        
        DO(nPrices,
           kF(date)[i] = reader->getDate(nPrices - i);
           kF(open)[i] = reader->getAskOpen(nPrices - i);
           kF(high)[i] = reader->getAskHigh(nPrices - i);
           kF(low)[i] = reader->getAskLow(nPrices - i);
           kF(close)[i] = reader->getAskClose(nPrices - i);
           kF(volume)[i] = reader->getVolume(nPrices - i));
        
        R xT(xD(headers, knk(6, date, open, high, low, close, volume)));
    } else {
        K headers = vector_to_k_list(std::vector<std::string> {"date", "ask" });
        K date = ktn(KZ, nPrices);
        K ask  = ktn(KF, nPrices);
        
        DO(nPrices,
           kF(date)[i] = reader->getDate(nPrices - i);
           kF(ask)[i] = reader->getAskOpen(nPrices - i));
        
        R xT(xD(headers, knk(2, date, ask)));
    }
}

void ForexConnect::setLogLevel(int level)
{
    O("Not implemented\n");
}
