/\d .fxcm

host:`$"http://www.fxcorporate.com/Hosts.jsp"
user:`D9382274001
pass:`5509
connection:`Demo

// Functions
so:`$"bin/Debug/libqfxcm"
/so:`$"bin/libqfxcm"
connect:so 2:(`connect;4)
disconnect:so 2:(`disconnect;1)
isconnected:so 2:(`isconnected;1)

gethistprices:so 2:(`gethistprices;4)

// Callbacks
onconnect:{show "callback: connected"; show 1+1}

// test
instrument:`$"EUR/USD"
timeframe:`H1
dtfrom:2016.03.01
dtto:.z.d

onhistprice:{show 10#x}