#include "ForexConnectClient.h"

#define Q(x, e) if(x) { krr((S)e); R 0; }

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

inline I uz(F f) { return 86400 * (f + 10957); } // unix from kdb+ datetime

struct tm *lt(int kd) { time_t t = uz(kd); return localtime(&t); }

static inline F toKTime(const double dt) { return dt - 36526; }

static double toOleTime(K t) {
    double d;
    struct tm tmBuf = *lt(t->i);
    CO2GDateUtils::CTimeToOleTime(&tmBuf, &d);
    return d;
}

K identity() { K id = ka(101); id->g = 0; return id; }

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
    Q(login->t != -11 || password->t != -11 || connection->t != -11 || url->t != -11, "type");
    client->login(login->s, password->s, connection->s, url->s);
    
    r0(login);
    r0(password);
    r0(connection);
    r0(url);
    
    R 0;
}

extern "C"
{
    K logout(K x) { client->logout(); R 0; }
    K isConnected(K x) { R kb(client->isConnected()); }
    K getAccountId(K x) { R ks((S)client->getAccountID().c_str()); }
    K getUsedMargin(K x) { R kf(client->getUsedMargin()); }
    K getBalance(K x) { R kf(client->getBalance()); }
    K getOffers(K x) { R map_to_k_dict(client->getOffers()); }
    K getTrades(K x) { R vector_to_k_list(client->getTrades()); }

    K getBid(K instrument)
    {
        Q(instrument->t != -11, "type");
        R kf(client->getBid(instrument->s));
    }
    
    K getAsk(K instrument)
    {
        Q(instrument->t != -11, "type");
        R kf(client->getAsk(instrument->s));
    }
    
    K openPosition(K instrument, K amount)
    {
        Q(instrument->t != -11 || amount->t != -7, "type");
        Q(amount->j == 0, "amount");
        R kb(client->openPosition(instrument->s, amount->j > 0 ? "B" : "S", amount->j));
    }
    
    K closePosition(K tradeID)
    {
        Q(tradeID->t != -11, "type");
        R kb(client->closePosition(tradeID->s));
    }
    
    K getHistoricalPrices(K instrument, K begin, K end, K timeframe)
    {
        Q(instrument->t != -11 || begin->t != -8 || end->t != -8 || timeframe->t != -11, "type");
        R vector_to_k_list(client->getHistoricalPrices(instrument->s,
                                                       toOleTime(begin),
                                                       toOleTime(end),
                                                       timeframe->s));
    }
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
    O("compiler flags » %-5s\n",        BUILD_COMPILER_FLAGS);
    O("\n");
    
    K keys = ktn(KS, 1);
    K vals = ktn(0, 1);
    
    kS(keys)[0] = ss((S) "version");
//    kS(keys)[1] = ss((S) "login");
//    kS(keys)[2] = ss((S) "logout");
//    kS(keys)[3] = ss((S) "isconnected");
//    kS(keys)[4] = ss((S) "getaccountid");
//    kS(keys)[5] = ss((S) "getusedmargin");
//    kS(keys)[6] = ss((S) "getbalance");
//    kS(keys)[7] = ss((S) "getoffers");
//    kS(keys)[8] = ss((S) "getbid");
//    kS(keys)[9] = ss((S) "getask");
//    kS(keys)[10] = ss((S) "gettrades");
//    kS(keys)[11] = ss((S) "openposition");
//    kS(keys)[12] = ss((S) "closeposition");
//    kS(keys)[13] = ss((S) "gethistprices");
    
    kK(vals)[0] = dl((void *) version, 1);
//    kK(vals)[1] = dl((void *) login, 4);
//    kK(vals)[2] = dl((void *) logout, 1);
//    kK(vals)[3] = dl((void *) isConnected, 1);
//    kK(vals)[4] = dl((void *) getAccountId, 4);
//    kK(vals)[5] = dl((void *) getUsedMargin, 1);
//    kK(vals)[6] = dl((void *) getBalance, 1);
//    kK(vals)[7] = dl((void *) getOffers, 1);
//    kK(vals)[8] = dl((void *) getBid, 2);
//    kK(vals)[9] = dl((void *) getAsk, 2);
//    kK(vals)[10] = dl((void *) getTrades, 1);
//    kK(vals)[11] = dl((void *) openPosition, 3);
//    kK(vals)[12] = dl((void *) closePosition, 1);
//    kK(vals)[13] = dl((void *) getHistoricalPrices, 4);
    
    R xD(keys, vals);
}
