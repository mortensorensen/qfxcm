#include "ForexConnectClient.h"
#include <string.h>

using namespace ForexConnect;

namespace
{
    // TODO: Type conversions
    template <class T>
    K vector_to_k_list(const std::vector<T> &vector) {
        typename std::vector<T>::const_iterator iter;
        K list;
        for (iter = vector.begin(); iter != vector.end(); ++iter) {
            ja(&list, &iter);
        }
        R list;
    }
    
    template <class Key, class Val>
    K map_to_k_dict(const std::map<Key, Val> &map) {
        typename std::map<Key, Val>::const_iterator iter;
        K keys, vals;
        for (iter = map.begin(); iter != map.end(); ++iter) {
            ja(&keys, const_cast<Key*>(&iter->first));
            ja(&vals, const_cast<Val*>(&iter->second));
        }
        R xD(keys, vals);
    }
}

LoginParams::LoginParams()
{
}

LoginParams::LoginParams(const std::string& login,
                         const std::string& password,
                         const std::string& connection,
                         const std::string& url)
    : mLogin(login),
      mPassword(password),
      mConnection(connection),
      mUrl(url)
{
}

bool LoginParams::areSet()
{
    return !(mLogin.empty() && mPassword.empty() && mConnection.empty() && mUrl.empty());
}

std::ostream& ForexConnect::operator<<(std::ostream &out, const ForexConnect::LoginParams &lp)
{
    out << "<'login': " << lp.mLogin
        << ", 'password': " << lp.mPassword
        << ", 'connection': " << lp.mConnection
        << ", 'url': " << lp.mUrl << ">";
    return out;
}

TradeInfo::TradeInfo()
    : mOpenRate(0.0),
      mOpenDate(std::time(0)),
      mGrossPL(0.0)
{
}

bool TradeInfo::operator==(const TradeInfo& other)
{
    return mTradeID == other.mTradeID;
}

bool TradeInfo::operator!=(const TradeInfo& other)
{
    return mTradeID != other.mTradeID;
}

std::ostream& ForexConnect::operator<<(std::ostream &out, const TradeInfo &ti)
{
    out << "<'instrument': " << ti.mInstrument
        << ", 'trade_id': " << ti.mTradeID
        << ", 'buy_sell': " << ti.mBuySell
        << ", 'open_rate': " << ti.mOpenRate
        << ", 'amount': " << ti.mAmount
        << ", 'open_date': " << ti.mOpenDate
        << ", 'gross_pl': " << ti.mGrossPL << ">";
    return out;
}

Prices::Prices()
    : mDate(std::time(0)),
      mOpen(0.0),
      mHigh(0.0),
      mLow(0.0),
      mClose(0.0)
{
}

Prices::Prices(DATE date, double value)
    : mDate(date),
      mOpen(value),
      mHigh(value),
      mLow(value),
      mClose(value)
{
}

Prices::Prices(DATE date,
               double open,
               double high,
               double low,
               double close)
    : mDate(date),
      mOpen(open),
      mHigh(high),
      mLow(low),
      mClose(close)
{
}

bool Prices::operator==(const Prices &other)
{
    return *this == other;
}

bool Prices::operator!=(const Prices &other)
{
    return !(*this == other);
}

std::ostream& ForexConnect::operator<<(std::ostream &out, Prices const& pr)
{
    out << "<'date': " << pr.mDate
        << ", 'open': " << pr.mOpen
        << ", 'high': " << pr.mHigh
        << ", 'low': " << pr.mLow
        << ", 'close': " << pr.mClose << ">";
    return out;
}

ForexConnectClient::ForexConnectClient()
    : mpSession(NULL),
    mpListener(NULL),
    mpResponseListener(NULL),
    mpLoginRules(NULL),
    mpAccountRow(NULL),
    mpResponseReaderFactory(NULL),
    mpRequestFactory(NULL)
{
}

ForexConnectClient::ForexConnectClient(const LoginParams& loginParams)
    : mLoginParams(loginParams),
    mpSession(NULL),
    mpListener(NULL),
    mpResponseListener(NULL),
    mpLoginRules(NULL),
    mpAccountRow(NULL),
    mpResponseReaderFactory(NULL),
    mpRequestFactory(NULL)
{
    init();
}

ForexConnectClient::ForexConnectClient(const std::string& login,
                                       const std::string& password,
                                       const std::string& connection,
                                       const std::string& url)
    : ForexConnectClient::ForexConnectClient(*new LoginParams(login, password, connection, url))
{
}

ForexConnectClient::~ForexConnectClient()
{
//    mpRequestFactory->release();
//    mpAccountRow->release();
//    mpLoginRules->release();
//    mpResponseReaderFactory->release();
//    mpSession->release();
//    mpResponseListener->release();
    if (mIsConnected) {
        logout();
    }
//    mpSession->unsubscribeSessionStatus(mpListener);
//    mpListener->release();
//    mpSession->release();
}

void ForexConnectClient::init()
{
    mpSession = CO2GTransport::createSession();
    mpListener = new SessionStatusListener(mpSession, false);
    mpSession->subscribeSessionStatus(mpListener);
    mpSession->useTableManager(Yes, 0);
}

bool ForexConnectClient::login()
{
    if (!mLoginParams.areSet()) {
        krr((S) "Login params not specified");
        return false;
    }
    return login(mLoginParams.mLogin,
                 mLoginParams.mPassword,
                 mLoginParams.mConnection,
                 mLoginParams.mUrl);
}

bool ForexConnectClient::login(const std::string& login,
                               const std::string& password,
                               const std::string& connection,
                               const std::string& url)
{
    mpListener->reset();
    mpSession->login(login.c_str(), password.c_str(), connection.c_str(), url.c_str());
    mIsConnected = mpListener->waitEvents() && mpListener->isConnected();
    if (!mIsConnected) {
        krr((S) "Login failed.");
        return false;
    }
    
    mpLoginRules = mpSession->getLoginRules();
    if (!mpLoginRules->isTableLoadedByDefault(Accounts)) {
        logout();
        krr((S) "Accounts table not loaded");
        return false;
    }
    
    auto response = mpLoginRules->getTableRefreshResponse(Accounts);
    if (!response) {
        logout();
        krr((S) "No response to refresh accounts table request");
        return false;
    }
    
    mpResponseReaderFactory = mpSession->getResponseReaderFactory();
    auto accountsResponseReader = mpResponseReaderFactory->createAccountsTableReader(response);
    mpAccountRow = accountsResponseReader->getRow(0);
    mAccountID = mpAccountRow->getAccountID();
    
    mpResponseListener = new ResponseListener(mpSession);
    mpSession->subscribeResponse(mpResponseListener);
    
    mpRequestFactory = mpSession->getRequestFactory();
    
    return mIsConnected;
}

void ForexConnectClient::logout()
{
    mpListener->reset();
    mpSession->logout();
    mpListener->waitEvents();
    mIsConnected = false;
}

bool ForexConnectClient::isConnected() const
{
    return mIsConnected;
}

std::string ForexConnectClient::getAccountID() const
{
    return mpAccountRow->getAccountID();
}

double ForexConnectClient::getUsedMargin() const
{
    return mpAccountRow->getUsedMargin();
}

double ForexConnectClient::getBalance()
{
    auto account = getAccount();
    return (!account) ? mpAccountRow->getBalance() : account->getBalance();
}

std::map<std::string, std::string> ForexConnectClient::getOffers()
{
    std::map<std::string, std::string> offers;
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = NULL;
    IO2GTableIterator it;
    while (offersTable->getNextRow(it, offerRow)) {
        offers[offerRow->getInstrument()] = offerRow->getOfferID();
        offerRow->release();
    }
    return offers;
}

double ForexConnectClient::getBid(const std::string &instrument)
{
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = NULL;
    IO2GTableIterator iter;
    while (offersTable->getNextRow(iter, offerRow)) {
        if (offerRow->getInstrument() == instrument) {
            const double bid = offerRow->getBid();
            offerRow->release();
            return bid;
        }
        offerRow->release();
    }
    krr((S) "Could not get offer table row");
    return NULL;
}

double ForexConnectClient::getAsk(const std::string &instrument)
{
    auto tableManager = getLoadedTableManager();
    auto offersTable = static_cast<IO2GOffersTable*>(tableManager->getTable(Offers));
    IO2GOfferTableRow *offerRow = NULL;
    IO2GTableIterator iter;
    while (offersTable->getNextRow(iter, offerRow)) {
        if (offerRow->getInstrument() == instrument) {
            const double ask = offerRow->getAsk();
            offerRow->release();
            return ask;
        }
        offerRow->release();
    }
    krr((S) "Could not get offer table row");
    return NULL;
}

std::vector<TradeInfo> ForexConnectClient::getTrades()
{
    std::vector<TradeInfo> trades;
    auto tableManager = getLoadedTableManager();
    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
    IO2GTradeTableRow* tradeRow = NULL;
    IO2GTableIterator tableIterator;
    std::map<std::string, std::string> offers = getOffers();
    while (tradesTable->getNextRow(tableIterator, tradeRow)) {
        TradeInfo trade;
        // TODO: FIX
        auto it = std::find_if(offers.begin(), offers.end(),
                               [tradeRow](const std::pair<std::string, std::string>& x) -> bool {
                                   return x.second == tradeRow->getOfferID();
                               });
        
//        Q(it == offers.end(), "Could not get offer table row");
        
        trade.mInstrument = it->first;
        trade.mTradeID = tradeRow->getTradeID();
        trade.mBuySell = tradeRow->getBuySell();
        trade.mOpenRate = tradeRow->getOpenRate();
        trade.mAmount = tradeRow->getAmount();
        trade.mOpenDate = tradeRow->getOpenTime();
        trade.mGrossPL = tradeRow->getGrossPL();
        trades.push_back(trade);
        tradeRow->release();
    }
    return trades;
}

bool ForexConnectClient::openPosition(const std::string &instrument,
                                      const std::string &buysell,
                                      int amount)
{
    if (buysell != O2G2::Buy && buysell != O2G2::Sell) {
        return false;
    }
    
    auto offers = getOffers();
    std::string offerID;
    auto offer_itr = offers.find(instrument);
    if (offer_itr != offers.end()) {
        offerID = offer_itr->second;
    } else {
        krr((S) "Could not find offer for for instrument ... "); // TODO: add instrument
        return false;
    }
    
    auto tradingSettingsProvider = mpLoginRules->getTradingSettingsProvider();
    auto iBaseUnitSize = tradingSettingsProvider->getBaseUnitSize(instrument.c_str(), mpAccountRow);
    auto valuemap = mpRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketOpen);
    valuemap->setString(AccountID, mAccountID.c_str());
    valuemap->setString(OfferID, offerID.c_str());
    valuemap->setString(BuySell, buysell.c_str());
    valuemap->setInt(Amount, amount * iBaseUnitSize);
    valuemap->setString(TimeInForce, O2G2::TIF::IOC);
    valuemap->setString(CustomID, "TrueMarketOrder");
    auto request = mpRequestFactory->createOrderRequest(valuemap);
    if (!request) {
        krr((S) mpRequestFactory->getLastError());
        return false;
    }
    mpResponseListener->setRequestID(request->getRequestID());
    mpSession->sendRequest(request);
    if (mpResponseListener->waitEvents()) {
        Sleep(1000); // Wait for the balance update
        O("Done!\n");
        return true;
    }
    krr((S) "Response waiting timeout expired");
    return false;
}

bool ForexConnectClient::closePosition(const std::string &tradeID)
{
    auto tableManager = getLoadedTableManager();
    auto tradesTable = static_cast<IO2GTradesTable*>(tableManager->getTable(Trades));
    IO2GTradeTableRow *tradeRow = NULL;
    IO2GTableIterator tableIterator;
    while (tradesTable->getNextRow(tableIterator, tradeRow)) {
        if (tradeID == tradeRow->getTradeID())
            break;
    }
    if (!tradeRow) {
        krr((S) "Could not found trade with ID = ..."); // TODO: add trade id
        return false;
    }
    auto valuemap = mpRequestFactory->createValueMap();
    valuemap->setString(Command, O2G2::Commands::CreateOrder);
    valuemap->setString(OrderType, O2G2::Orders::TrueMarketClose);
    valuemap->setString(AccountID, mAccountID.c_str());
    valuemap->setString(OfferID, tradeRow->getOfferID());
    valuemap->setString(TradeID, tradeID.c_str());
    valuemap->setString(BuySell, (strcmp(tradeRow->getBuySell(), O2G2::Buy) == 0) ? O2G2::Sell : O2G2::Buy);
    tradeRow->release();
    valuemap->setInt(Amount, tradeRow->getAmount());
    valuemap->setString(CustomID, "CloseMarketOrder");
    auto request = mpRequestFactory->createOrderRequest(valuemap);
    if (!request) {
        krr((S) mpRequestFactory->getLastError());
        return false;
    }
    mpResponseListener->setRequestID(request->getRequestID());
    mpSession->sendRequest(request);
    if (mpResponseListener->waitEvents()) {
        Sleep(1000); // Wait for the balance update
        O("Done!\n");
        return true;
    }
    krr((S) "Response waiting timeout expired");
    return false;
}

std::vector<Prices> ForexConnectClient::getHistoricalPrices(const std::string &instrument,
                                                            const DATE from,
                                                            const DATE to,
                                                            const std::string& timeFrame)
{
    std::vector<Prices> prices;
    auto timeframeCollection = mpRequestFactory->getTimeFrameCollection();
    auto timeframe = timeframeCollection->get(timeFrame.c_str());
    if (!timeframe) {
        krr((S) "Timeframe ... is incorrect"); // TODO: add timeframe
        return prices;
    }
    auto request = mpRequestFactory->createMarketDataSnapshotRequestInstrument(instrument.c_str(),
                                                                               timeframe,
                                                                               timeframe->getQueryDepth());
    DATE first = to;
    do
    {
        mpRequestFactory->fillMarketDataSnapshotRequestTime(request, from, first, false);
        mpResponseListener->setRequestID(request->getRequestID());
        mpSession->sendRequest(request);
        if (!mpResponseListener->waitEvents()) {
            krr((S) "Response waiting timeout expired");
            return prices;
        }
        // shift "to" bound to oldest datetime of returned data
        auto response = mpResponseListener->getResponse();
        if (response && response->getType() == MarketDataSnapshot) {
            auto reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
            if (reader->size() > 0) {
                if (fabs(first - reader->getDate(0)) > 0.0001)
                    first = reader->getDate(0); // earliest datetime of return data
                else
                    break;
            } else {
                O("0 rows received\n");
                break;
            }
            auto px = getPricesFromResponse(response);
            prices.insert(prices.end(), px.begin(), px.end());
        } else {
            break;
        }
    } while (first - from > 0.0001);

    return prices;
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
    return NULL;
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
        krr((S) "Cannot refresh all tables of table manager");
        return NULL;
    }
    return tableManager.Detach();
}

std::vector<Prices> ForexConnectClient::getPricesFromResponse(IO2GResponse *response)
{
    std::vector<Prices> prices;
    if (!response || response->getType() != MarketDataSnapshot)
        return prices;
    
    O("Request with RequestID='%s' is completed:\n", response->getRequestID());
    
    auto reader = mpResponseReaderFactory->createMarketDataSnapshotReader(response);
    if (!reader)
        return prices;
    
    for (int i = reader->size() - 1; i >= 0; i--) {
        if (reader->isBar()) {
            prices.push_back(Prices(reader->getDate(i),
                                    reader->getAskOpen(i),
                                    reader->getAskHigh(i),
                                    reader->getAskLow(i),
                                    reader->getAskClose(i)));
        } else {
            prices.push_back(Prices(reader->getDate(i), reader->getAsk(i)));
        }
    }
    return prices;
}

void ForexConnect::setLogLevel(int level)
{
    O("Not implemented\n");
}
