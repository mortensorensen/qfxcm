/\d .fxcm

args:.Q.opt .z.x

host:`$"http://www.fxcorporate.com/Hosts.jsp"
user:first `$args[`user]
pass:first `$args[`pass]
connection:`Demo

// Functions
so:`$"bin/Debug/libqfxcm"
/so:`$"bin/libqfxcm"
connect:so 2:(`connect;4)
disconnect:so 2:(`disconnect;1)
isconnected:so 2:(`isconnected;1)

gethistprices:so 2:(`gethistprices;4)

out:{-1(string .z.Z)," ",x;}

// Callbacks
onconnecting:{out"status::connecting"}
onconnected:{out"status::connected"}
ondisconnecting:{out"status::disconnecting"}
ondisconnected:{out"status::disconnected"}
onreconnecting:{out"status::reconnecting"}
onsessionlost:{out"status::session lost"}
ontradingsessionrequested:{out"status::trading session requested"}
onloginfailed:{out"status::the specified sub session id was not found"}


// test
instrument:`$"EUR/USD"
timeframe:`H1
dtfrom:2016.03.01
dtto:.z.d

onhistprice:{show 10#x}