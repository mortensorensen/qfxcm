#ifndef MarketDataSnapshotAccumulator_h
#define MarketDataSnapshotAccumulator_h

#include "stdafx.h"
#include "ResponseListener.h"
#include "CommonSources.h"
#include "Helpers.h"

class MarketDataSnapshotAccumulator
{
public:
    MarketDataSnapshotAccumulator();
    virtual ~MarketDataSnapshotAccumulator();
    
    void addReader(O2G2Ptr<IO2GMarketDataSnapshotResponseReader> reader);
    const K getTable();
    
private:
    std::vector<O2G2Ptr<IO2GMarketDataSnapshotResponseReader> > mReaders;
    
    const J totalSize();
    
    const K getTickTable();
    const K getBarTable();
};

#endif