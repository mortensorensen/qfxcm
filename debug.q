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
/ .fxcm.gethistprices[instrument;begin;end;timeframe]
/ .fxcm.truemarketopen[instrument;amount]
.fxcm.subscribeoffers[instrument]


/ select from t
/ .fxcm.gettrades[]
/ .fxcm.openposition[`$"AUD/NZD";100]
/ .fxcm.getoffers[]


/ .fxcm.truemarketopen[instrument;amount]

/ .fxcm.testdict[1 2 3!4 5 6]

/ .fxcm.createorderstub[]