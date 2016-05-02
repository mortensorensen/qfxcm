.fxcm:(`:./build/Debug/qfxcm.0.0.1 2:(`LoadLibrary;1))`
.fxcm.fixargs:{(),$[10h=type x;enlist x;x]}
.fxcm.onrecv:{[fname;args] $[null code:.fxcm.callbacks[fname];.fxcm.unknown . (fname;args);code . args]}
.fxcm.callbacks:()!()
.fxcm.unknown:{[fname;args] -1(string .z.Z)," unknown function ",(string fname),", args: ";0N!args}
.fxcm.reg:{[fname;code] @[`.fxcm.callbacks;fname;:;code];}
.fxcm.dreg:{[fname] .fxcm.callbacks _::fname;}

system"l ",getenv[`QFXCMROOT],"/tags.q"

.fxcm.getofferid:{$[null id:.fxcm.getoffers[][x];'`instrument;id]}

.fxcm.createorderstub:{
  msg:enlist[]!enlist[];
	msg[.fxcm.params.Command]: .fxcm.commands.CreateOrder;
	msg[.fxcm.params.AccountID]: .fxcm.getaccountid[];
	:msg
 }

.fxcm.truemarketopen:{[instrument;amount]
	msg:.fxcm.createorderstub[];
	msg[.fxcm.params.OrderType]: .fxcm.orders.TrueMarketOpen;
	msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	msg[.fxcm.params.BuySell]: $[amount>0;.fxcm.Buy;.fxcm.Sell];
	msg[.fxcm.params.Amount]: amount * .fxcm.getbaseunitsize[instrument];
	msg[.fxcm.params.TimeInForce]: .fxcm.tif.IOC;
	msg[.fxcm.params.CustomID]: `TrueMarketOrder;
	.fxcm.send msg _(::);
 }

.fxcm.truemarketclose:{[tradeid]
	msg:.fxcm.createorderstub[];
	msg[.fxcm.params.OrderType]: .fxcm.orders.TrueMarketClose;
	msg[.fxcm.params.OfferID]: 1;
	msg[.fxcm.params.TradeID]: 1;
	msg[.fxcm.params.BuySell]: 1;
	msg[.fxcm.params.Amount]: 1;
	msg[.fxcm.params.CustomID]: `TrueMarketClose;
	.fxcm.send msg _(::);
 }

.fxcm.sub:{[instrument]
	msg:()!();
	msg[.fxcm.params.Command]: .fxcm.commands.SetSubscriptionStatus;
	msg[.fxcm.params.SubscriptionStatus]: .fxcm.subscribtionstatus.Tradable;
	msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	.fxcm.send msg;
 }

.fxcm.unsub:{[instrument]
	msg:()!();
	msg[.fxcm.params.Command]: .fxcm.commands.SetSubscriptionStatus;
	msg[.fxcm.params.SubscriptionStatus]: .fxcm.subscribtionstatus.Disable;
	msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	.fxcm.send msg;
	/ :msg
 }

.fxcm.suball:{.fxcm.sub each key .fxcm.getoffers[]}
.fxcm.unsuball:{.fxcm.unsub each key .fxcm.getoffers[]}

/ out:{-1(string .z.Z)," ",x;}
out:{0N!enlist .z.Z,enlist x}

.fxcm.quotes:([] date:`datetime$(); sym:`symbol$(); bid:`float$(); ask:`float$())

/ Default callbacks
.fxcm.reg[`connecting]{out"status::connecting"}
.fxcm.reg[`connected]{out"status::connected"}
.fxcm.reg[`disconnecting]{out"status::disconnecting"}
.fxcm.reg[`disconnected]{out"status::disconnected"}
.fxcm.reg[`reconnecting]{out"status::reconnecting"}
.fxcm.reg[`sessionlost]{out"status::session lost"}
.fxcm.reg[`tradingsessionrequested]{out"status::trading session requested"}
.fxcm.reg[`loginfailed]{[msg] out"status::the specified sub session id was not found: ",err}

.fxcm.reg[`histprices]{show x}
/ .fxcm.reg[`onoffer]{[time;sym;bid;ask] out"onoffer, time: ",(string time)," sym: ",(string sym)," bid: ",(string bid)," ask: ",string ask}
.fxcm.reg[`onoffer]{[time;sym;bid;ask] `.fxcm.quotes insert (time;sym;bid;ask); out (time;sym;bid;ask)}
/ .fxcm.reg[`onoffer]{[time;sym;bid;ask] a::1}
/ .fxcm.reg[`onoffer]{a::1}
/ .fxcm.reg[`onoffer]{[time;sym;bid;ask] out"onoffer"}

onhistprice:{show 10#x}
