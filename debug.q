system"l /Users/morten/Sandbox/algotrading/fxcm/qfxcm/qfxcm.q"

args:.Q.opt .z.x
host:`$"http://www.fxcorporate.com/Hosts.jsp"
user:first `$args[`user]
pass:first `$args[`pass]
connection:`Demo

instrument:`$"EUR/USD"
begin:`datetime$2016.04.01
end:`datetime$.z.d
timeframe:`M1
amount:100

// debug
.fxcm.login[user;pass;connection;host]
/ .fxcm.subscribeoffers[`$"EUR/USD"]
/ show t:.fxcm.gethistprices[sym;begin;end;timeframe]
/ .fxcm.getoffers[]
/ show .fxcm.testoletime[`datetime$.z.d]

/ select from t
/ .fxcm.gettrades[]
/ .fxcm.openposition[`$"AUD/NZD";100]
/ .fxcm.getoffers[]


.fxcm.truemarketopen[instrument;amount]

/ .fxcm.testdict[1 2 3!4 5 6]

/ .fxcm.createorderstub[]