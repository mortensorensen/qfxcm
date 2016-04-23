#include "ForexConnectClient.h"
#include "Helpers.h"

//using namespace ForexConnect;

ForexConnect::ForexConnectClient* client;

// TODO: Implement MSVC equivalent
__attribute__((constructor))
static void initialize_api() {
    client = new ForexConnect::ForexConnectClient();
}

__attribute__((destructor))
static void destroy_api() {
    delete client;
}

extern "C"
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

extern "C"
K login(K login, K password, K connection, K url)
{
    R client->login(login, password, connection, url);
}

extern "C"
{
    K logout(K x) { client->logout(); R 0; }
    K isConnected(K x) { R client->isConnected(); }
    K getAccountId(K x) { R client->getAccountID(); }
    K getUsedMargin(K x) { R client->getUsedMargin(); }
    K getBalance(K x) { R client->getBalance(); }
    K getOffers(K x)
    {
        auto offers = client->getOffers();
        K keys = ktn(KS, 0);
        K vals = ktn(KS, 0);
        for (auto it = offers.begin(); it != offers.end(); ++it) {
            js(&keys, ss((S)it->first.c_str()));
            js(&vals, ss((S)it->second.c_str()));
        }
        R xD(keys, vals);
    }
    K getTrades(K x) { R client->getTrades(); }
    K getBid(K instrument) { R client->getBid(instrument); }
    K getAsk(K instrument) { R client->getAsk(instrument); }
    
    K openPosition(K instrument, K amount)
    {
        R client->openPosition(instrument, amount);
    }
    
    K closePosition(K tradeID)
    {
        Q(tradeID->t != -KS, "type");
        R client->closePosition(tradeID);
    }
    
    K getHistoricalPrices(K instrument, K begin, K end, K timeframe)
    {
        Q(instrument->t != -KS || begin->t != -KD || end->t != -KD || timeframe->t != -KS, "type");
        R client->getHistoricalPrices(instrument->s,
                                      toCOleTime(begin),
                                      toCOleTime(end),
                                      timeframe->s);
    }
    K subscribeOffers(K instrument) { R client->subscribeOffers(instrument); }
    K unsubscribeOffers(K x) { R client->unsubscribeOffers(x); }
    K getServerTime(K x) { R client->getservertime(x); }
}


extern "C"
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
    O("compiler flags »%-5s\n",        BUILD_COMPILER_FLAGS);
    O("\n");
    
    auto map = std::map<const char*, std::pair<void*, unsigned short> > {
        { "version",        { (void*)version,           1 } },
        { "login",          { (void*)login,             4 } },
        { "logout",         { (void*)logout,            1 } },
        { "isconnected",    { (void*)isConnected,       1 } },
        { "getaccountid",   { (void*)getAccountId,      1 } },
        { "getusedmargin",  { (void*)getUsedMargin,     1 } },
        { "getbalance",     { (void*)getBalance,        1 } },
        { "getoffers",      { (void*)getOffers,         1 } },
        { "getbid",         { (void*)getBid,            1 } },
        { "getask",         { (void*)getAsk,            1 } },
        { "gettrades",      { (void*)getTrades,         1 } },
        { "openposition",   { (void*)openPosition,      2 } },
        { "closeposition",  { (void*)closePosition,     1 } },
        { "gethistprices",  { (void*)getHistoricalPrices,  4 } },
        { "subscribeoffers",   { (void*)subscribeOffers,   1 } },
        { "unsubscribeoffers", { (void*)unsubscribeOffers, 1 } },
        { "getservertime",  { (void*)getServerTime,     1 } }
    };
    
    K keys = ktn(KS, 0);
    K vals = knk(0);
    for (auto it = map.begin(); it != map.end(); ++it) {
        js(&keys, ss((S)it->first));
        jk(&vals, dl(it->second.first, it->second.second));
    }
    
    R xD(keys, vals);
}
