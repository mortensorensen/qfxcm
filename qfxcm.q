.fxcm:(`:./build/Debug/qfxcm.0.0.1 2:(`LoadLibrary;1))`

args:.Q.opt .z.x

host:`$"http://www.fxcorporate.com/Hosts.jsp"
user:first `$args[`user]
pass:first `$args[`pass]
connection:`Demo

out:{-1(string .z.Z)," ",x;}

.fxcm.onrecv:{[x]
	show x;
	}

// Callbacks
.fxcm.onconnecting:{out"status::connecting"}
.fxcm.onconnected:{out"status::connected"}
.fxcm.ondisconnecting:{out"status::disconnecting"}
.fxcm.ondisconnected:{out"status::disconnected"}
.fxcm.onreconnecting:{out"status::reconnecting"}
.fxcm.onsessionlost:{out"status::session lost"}
.fxcm.ontradingsessionrequested:{out"status::trading session requested"}
.fxcm.onloginfailed:{out"status::the specified sub session id was not found"}

.fxcm.onoffer:{out"offer received"}
.fxcm.onlevel2data:{out"level 2 received"}

// test
instrument:`$"EUR/USD"
timeframe:`H1
dtfrom:2016.03.01
dtto:.z.d

onhistprice:{show 10#x}