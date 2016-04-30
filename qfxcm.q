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
  order:enlist[]!enlist[];
	order[.fxcm.params.Command]: .fxcm.commands.CreateOrder;
	order[.fxcm.params.AccountID]: .fxcm.getaccountid[];
	:order
 }

.fxcm.truemarketopen:{[instrument;amount]
	order:.fxcm.createorderstub[];
	order[.fxcm.params.OrderType]: .fxcm.orders.TrueMarketOpen;
	order[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	order[.fxcm.params.BuySell]: $[amount>0;.fxcm.Buy;.fxcm.Sell];
	order[.fxcm.params.Amount]: amount * .fxcm.getbaseunitsize[instrument];
	order[.fxcm.params.TimeInForce]: .fxcm.tif.IOC;
	order[.fxcm.params.CustomID]: `TrueMarketOrder;
	.fxcm.send order _(::)
 }

.fxcm.truemarketclose:{[tradeid]
	order:.fxcm.createorderstub[];
	order[.fxcm.params.OrderType]: .fxcm.orders.TrueMarketClose;
	order[.fxcm.params.OfferID]: 1;
	order[.fxcm.params.TradeID]: 1;
	order[.fxcm.params.BuySell]: 1;
	order[.fxcm.params.Amount]: 1;
	order[.fxcm.params.CustomID]: `TrueMarketClose;
	.fxcm.send order _(::)
 }

.fxcm.sub:{[instrument]
	msg:()!();
	msg[.fxcm.params.Command]: .fxcm.commands.SetSubscriptionStatus;
	msg[.fxcm.params.SubscriptionStatus]: .fxcm.subscribtionstatus.Tradable;
	msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	.fxcm.send msg;
 }

.fxcm.unsub:{[instrument]
	/ msg:()!();
	msg:enlist[]!enlist[];
	msg[.fxcm.params.Command]: .fxcm.commands.SetSubscriptionStatus;
	msg[.fxcm.params.SubscriptionStatus]: .fxcm.subscribtionstatus.Disable;
	msg[.fxcm.params.OfferID]: .fxcm.getofferid[instrument];
	.fxcm.send msg _(::);
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
