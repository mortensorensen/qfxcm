system"l /Users/morten/Sandbox/algotrading/fxcm/qfxcm/qfxcm.q"

args:.Q.opt .z.x
user:first `$args[`user]
pass:first `$args[`pass]
connection:`Demo
host:`$"http://www.fxcorporate.com/Hosts.jsp"

.fxcm.setloginparams[user;pass;connection;host]

instrument:`$"USD/JPY"
begin:`datetime$2016.04.01
end:`datetime$.z.d
timeframe:`H1
amount:100

/ .fxcm.version[]
.fxcm.connect[]
/ .fxcm.disconnect[]
/ .fxcm.isconnected[]
/ .fxcm.getaccountid[]
/ .fxcm.getusedmargin[]
/ .fxcm.getbalance[]
/ .fxcm.getoffers[]
/ .fxcm.getbid[instrument]
/ .fxcm.getask[instrument]
/ .fxcm.gettrades[]
/ .fxcm.getservertime[]
/ .fxcm.getbaseunitsize[instrument]
/ show hist:.fxcm.gethistprices[instrument;begin;end;timeframe]
/ .fxcm.truemarketopen[instrument;amount]
/ .fxcm.subscribeoffers[instrument]
/ .fxcm.unsubscribeoffers[]
/ .fxcm.sub `$"AUD/CAD"
/ .fxcm.unsub `$"AUD/CAD"
/ .fxcm.quotes

/ select from t
/ .fxcm.gettrades[]
/ .fxcm.openposition[`$"AUD/NZD";100]
/ .fxcm.getoffers[]


/ .fxcm.truemarketopen[instrument;amount]

/ msg:enlist[]!enlist[];
/ msg[.fxcm.params.Command]: .fxcm.commands.CreateOrder;
/ msg[.fxcm.params.OrderType]: .fxcm.orders.TrueMarketOpen;
/ msg[.fxcm.params.AccountID]: .fxcm.getaccountid[];
/ msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
/ msg[.fxcm.params.BuySell]: $[amount>0;.fxcm.Buy;.fxcm.Sell];
/ msg[.fxcm.params.Amount]: amount * .fxcm.getbaseunitsize[instrument];
/ / msg[.fxcm.params.TimeInForce]: .fxcm.tif.IOC;
/ msg[.fxcm.params.CustomID]: `TestTrueMarketOrder;
/ .fxcm.send msg _(::)


