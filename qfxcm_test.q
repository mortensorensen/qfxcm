.tst.desc["QFXCM"]{
	before{
		`func mock `test;
		`code mock {x+y};
		`args mock (1;2);
	};
	should["fail when using non symbols as connection parameters"]{
		mustthrow["type";(.fxcm.connect;string host;string user;string pass;string connection)];
	};
	should["fail when using connection different than Real or Demo"]{
	 	mustthrow["connection";(.fxcm.connect;host;user;pass;`abcd)];
	 };
	should["not fail when receing an unknown function"]{
		mustnotthrow[();(`.fxcm.onrecv;`unknownfunc;enlist 123)];
	};
	should["return correct result for registered functions"]{
		.fxcm.reg[`func;`code];
		.fxcm.onrecv[`func;args] musteq 3;
	};
	alt{
		before {
			`func mock `emptyArgFunc;
			`code mock "test output";
			`args mock ();
		};
		should["allow empty list arguments to onrecv"]{
			.fxcm.reg[`func;`code];
			mustnotthrow[();(`.fxcm.onrecv;(`func;`args))];
		};
	};
 };