#ifndef MarketDataAccumulator_h
#define MarketDataAccumulator_h

#include "stdafx.h"
#include "ResponseListener.h"
#include "CommonSources.h"
#include "k.h"

class MarketDataAccumulator
{
public:
    void addReader(O2G2Ptr<IO2GMarketDataSnapshotResponseReader> *reader);
    const K getTable();
    
private:
    std::vector<O2G2Ptr<IO2GMarketDataSnapshotResponseReader*> > mReaders;
    
protected:
    virtual ~MarketDataAccumulator();
};

#endif /* MarketDataAccumulator_h */
