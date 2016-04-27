#include "qforexconnect.h"
#include "Helpers.h"

IO2GSession* mpSession;
SessionStatusListener* mpListener;
ResponseListener* mpResponseListener;
IO2GLoginRules* mpLoginRules;
IO2GAccountRow* mpAccountRow;
IO2GResponseReaderFactory* mpResponseReaderFactory;
IO2GRequestFactory* mpRequestFactory;
std::string mAccountID;
TableListener* mpTableListener;
LoginParams* loginParams;

// TODO: Implement MSVC equivalent
__attribute__((constructor))
static void initialize_api() {
    mpSession = CO2GTransport::createSession();
    mpListener = new SessionStatusListener(mpSession, false, "", "");
    mpSession->subscribeSessionStatus(mpListener);
    mpSession->useTableManager(Yes, 0);
    
    mpResponseListener = new ResponseListener(mpSession);
    mpSession->subscribeResponse(mpResponseListener);
    loginParams = new LoginParams();
}

__attribute__((destructor))
static void destroy_api() {
    if (mpRequestFactory) mpRequestFactory->release();
    if (mpAccountRow) mpAccountRow->release();
    if (mpLoginRules) mpLoginRules->release();
    if (mpResponseReaderFactory) mpResponseReaderFactory->release();
    if (mpResponseListener) mpResponseListener->release();
    if (mpListener->isConnected()) disconnect((K)0);
    mpSession->unsubscribeSessionStatus(mpListener);
    mpListener->release();
    mpSession->release();
    delete loginParams;
}

K version(K x)
{
    K keys = ktn(KS, 4);
    K vals = ktn(KS, 4);
    
    kS(keys)[0] = ss((S) "release");
    kS(keys)[1] = ss((S) "os");
    kS(keys)[2] = ss((S) "forexconnect");
    kS(keys)[3] = ss((S) "kx");
    
    kS(vals)[0] = ss((S) BUILD_PROJECT_VERSION);
    kS(vals)[1] = ss((S) BUILD_OPERATING_SYSTEM);
    kS(vals)[2] = ss((S) BUILD_FOREXCONNECT_VER);
    kS(vals)[3] = ss((S) BUILD_KX_VER);
    
    R xD(keys, vals);
}

K LoadLibrary(K x)
{
    O("\n");
    O("%s:\n",                          PROGRAM_NAME);
    O("release » %-5s\n",               BUILD_PROJECT_VERSION);
    O("os » %-5s\n",                    BUILD_OPERATING_SYSTEM);
    O("arch » %-5s\n",                  BUILD_PROCESSOR);
    O("git commit » %-5s\n",            BUILD_GIT_SHA1);
    O("git commit datetime » %-5s\n",   BUILD_GIT_DATE);
    O("kdb compatibility » %s.x\n",     BUILD_KX_VER);
    O("compiler flags »%-5s\n",         BUILD_COMPILER_FLAGS);
    O("\n");
    
    auto map = std::map<const char*, std::pair<void*, unsigned short> > {
        { "version",        { (void*)version,           1 } },
        { "setloginparams", { (void*)setLoginParams,    4 } },
        { "connect",        { (void*)connect,           1 } },
        { "disconnect",     { (void*)disconnect,        1 } },
        { "isconnected",    { (void*)isConnected,       1 } },
        { "getaccountid",   { (void*)getAccountId,      1 } },
        { "getusedmargin",  { (void*)getUsedMargin,     1 } },
        { "getbalance",     { (void*)getBalance,        1 } },
        { "getoffers",      { (void*)getOffers,         1 } },
        { "getbid",         { (void*)getBid,            1 } },
        { "getask",         { (void*)getAsk,            1 } },
        { "gettrades",      { (void*)getTrades,         1 } },
        { "sendorder",      { (void*)sendOrder,         1 } },
        { "gethistprices",  { (void*)getHistoricalPrices,  4 } },
        { "subscribeoffers",   { (void*)subscribeOffers,   1 } },
        { "unsubscribeoffers", { (void*)unsubscribeOffers, 1 } },
        { "getservertime",  { (void*)getServerTime,     1 } },
        { "getbaseunitsize",   { (void*)getBaseUnitSize,   1 } }
    };
    
    K keys = ktn(KS, 0);
    K vals = knk(0);
    for (auto it = map.begin(); it != map.end(); ++it) {
        js(&keys, ss((S)it->first));
        jk(&vals, dl(it->second.first, it->second.second));
    }
    
    R xD(keys, vals);
}

void LoginParams::set(const std::string &login, const std::string &password, const std::string &connection, const std::string &url)
{
    mLogin = login;
    mPassword = password;
    mConnection = connection;
    mUrl = url;
}

bool LoginParams::areSet()
{
    return !mLogin.empty() && !mPassword.empty() && !mConnection.empty() && !mUrl.empty();
}

K setLoginParams(K login, K password, K connection, K url)
{
    Q(login->t != -KS || password->t != -KS || connection->t != -KS || url->t != -KS, "type");
    Q(strcasecmp(connection->s, "Real") != 0 &&
      strcasecmp(connection->s, "Demo") != 0,
      "connection");
    loginParams->set(login->s, password->s, connection->s, url->s);
    R 0;
}

K connect(K x)
{
    Q(!loginParams->areSet(), "login params not set");
    Q(mpListener->isConnected(), "already connected");
    
    mpListener->reset();
    mpSession->login(loginParams->mLogin.c_str(),
                     loginParams->mPassword.c_str(),
                     loginParams->mUrl.c_str(),
                     loginParams->mConnection.c_str());
    Q(!(mpListener->waitEvents() && mpListener->isConnected()), "Couldn't connect");
    
    mpLoginRules = mpSession->getLoginRules();
    if (!mpLoginRules->isTableLoadedByDefault(Accounts)) {
        mpSession->logout();
        R krr((S) "Accounts table not loaded");
    }
    
    auto response = mpLoginRules->getTableRefreshResponse(Accounts);
    if (!response) {
        mpSession->logout();
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

K disconnect(K x)
{
    if (!mpListener->isConnected()) {
        O("already logged out\n");
        R (K)0;
    }
    mpListener->reset();
    mpSession->logout();
    mpListener->waitEvents();
    R 0;
}

K isConnected(K x)
{
    R kb(mpListener->isConnected());
}

K getAccountId(K x)
{
    if (!mpListener->isConnected())
        connect((K)0);
    R ks((S)mpAccountRow->getAccountID());
}

K getUsedMargin(K x)
{
    if (!mpListener->isConnected())
        connect((K)0);
    R kf(mpAccountRow->getUsedMargin());
}

K getBalance(K x)
{
    if (!mpListener->isConnected())
        connect((K)0);
    auto account = getAccount();
    R kf((!account) ? mpAccountRow->getBalance() : account->getBalance());
}

K getOffers(K x)
{
    auto offersMap = getOffersMap();
    K keys = ktn(KS, 0);
    K vals = ktn(KS, 0);
    for (auto it = offersMap.begin(); it != offersMap.end(); ++it) {
        js(&keys, ss((S)it->first.c_str()));
        js(&vals, ss((S)it->second.c_str()));
    }
    R xD(keys, vals);
}

K getBid(K instrument)
{
    Q(instrument->t != -KS, "type");
    if (!mpListener->isConnected())
        connect((K)0);
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

K getAsk(K instrument)
{
    Q(instrument->t != -KS, "type");
    if (!mpListener->isConnected())
        connect((K)0);
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

K getTrades(K x)
{
    if (!mpListener->isConnected())
        connect(x);
    auto tableManager = getLoadedTableManager();
    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
    IO2GTradeTableRow* tradeRow = nullptr;
    IO2GTableIterator tableIterator;
    auto offers = getOffersMap();
    
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

//K openPosition(K& dict)
//{
//    if (!mpListener->isConnected())
//        connect((K)0);
//    //    Q(instrument->t != -KS || amount->t != -KJ, "type");
//    //    if (amount->j == 0) R 0;
//    
//    // Find offer ID
//    //    auto offers = getOffers();
//    //    auto offerItr = offers.find(instrument->s);
//    //    Q(offerItr == offers.end(), "instrument");
//    //    std::string offerID = offerItr->second;
//    
//    //    auto tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
//    //    uint iBaseUnitSize = tradingSettingsProvider->getBaseUnitSize(instrument->s, mpAccountRow);
//    
//    auto valuemap = createValueMap(dict);
//    Q(!valuemap, "error");
//    auto request = mpRequestFactory->createOrderRequest(valuemap);
//    if (!request) {
//        R krr((S)mpRequestFactory->getLastError());
//    }
//    mpResponseListener->setRequestID(request->getRequestID());
//    mpSession->sendRequest(request);
//    if (mpResponseListener->waitEvents()) {
//        Sleep(1000); // Wait for the balance update
//        O("Done!\n");
//        R kb(true);
//    }
//    R krr((S) "Response waiting timeout expired");
//}

K sendOrder(K dict)
{
    if (!mpListener->isConnected())
        connect((K)0);
    auto valuemap = createValueMap(dict);
    auto request = mpRequestFactory->createOrderRequest(valuemap);
    Q(!request, mpRequestFactory->getLastError());
    
    mpResponseListener->setRequestID(request->getRequestID());
    mpSession->sendRequest(request);
    if (mpResponseListener->waitEvents()) {
        Sleep(1000); // Wait for the balance update
        O("Order placed\n");
        R kb(true);
    }
    R krr((S) "Response waiting timeout expired");
}

K getHistoricalPrices(K instrument, K from, K to, K timeFrame)
{
    Q(!(instrument->t == -KS &&
        (from->t == -KZ || from->t == -KP) &&
        (to->t == -KZ || to->t == -KP) &&
        timeFrame->t == -KS), "type");
    if (!mpListener->isConnected())
        connect((K)0);
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
    
    std::vector<std::string> strHeaders = {"date", "bidopen", "bidhigh", "bidlow", "bidclose", "askopen", "askhigh", "asklow", "askclose", "volume" };
    K headers = ktn(KS, strHeaders.size());
    DO(strHeaders.size(), kS(headers)[i] = ss((S)strHeaders[i].c_str()));
    
    // TODO: Implement for ticks
    
    R xT(xD(headers, prices));
}

K subscribeOffers(K instrument)
{
    Q(instrument->t != -KS, "type");
    if (!mpListener->isConnected())
        connect((K)0);
    
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

K unsubscribeOffers(K x)
{
    if (!mpListener->isConnected())
        connect((K)0);
    mpTableListener->unsubscribeEvents(getLoadedTableManager());
    R 0;
}

K getServerTime(K x)
{
    if (!mpListener->isConnected())
        connect((K)0);
    R toKTime(mpSession->getServerTime());
}

K getBaseUnitSize(K instrument)
{
    Q(instrument->t != -KS, "type");
    if (!mpListener->isConnected())
        connect((K)0);
    O2G2Ptr<IO2GTradingSettingsProvider> tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
    int baseUnitSize = tradingSettingsProvider->getBaseUnitSize(instrument->s, mpAccountRow);
    Q(baseUnitSize == -1, "instrument");
    R kj(baseUnitSize);
}

static std::map<std::string, std::string> getOffersMap()
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

static IO2GAccountTableRow* getAccount()
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

static IO2GTableManager* getLoadedTableManager()
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

static K getPricesFromResponse(IO2GResponse *response)
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

static IO2GValueMap* createValueMap(K &dict)
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
            O("Type %i not recognized for key %i\n", val->t, (int)key);
            hasError = true;
            break;
        }
    }
    return hasError ? nullptr : valuemap;
}
