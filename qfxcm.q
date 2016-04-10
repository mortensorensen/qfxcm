.fxcm:(`:./build/Debug/qfxcm.0.0.1 2:(`LoadLibrary;1))`
.fxcm.fixargs:{(),$[10h=type x;enlist x;x]}
.fxcm.onrecv:{[fname;args] $[null code:.fxcm.callbacks[fname];.fxcm.unknown . (fname;args);code . args]}
.fxcm.callbacks:()!()
.fxcm.unknown:{[fname;args] -1(string .z.Z)," unknown function ",(string fname),", args: ";0N!args}
.fxcm.reg:{[fname;code] @[`.fxcm.callbacks;fname;:;code];}
.fxcm.dreg:{[fname] .fxcm.callbacks _::fname;}


out:{-1(string .z.Z)," ",x;}

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

// test
instrument:`$"EUR/USD"
timeframe:`H1
dtfrom:2016.03.01
dtto:.z.d

onhistprice:{show 10#x}

args:.Q.opt .z.x
host:`$"http://www.fxcorporate.com/Hosts.jsp"
user:first `$args[`user]
pass:first `$args[`pass]
connection:`Demo
