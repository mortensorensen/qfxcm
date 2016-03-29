.tst.desc["QFXCM"]{
	should["fail when using non symbols as connection parameters"]{
		mustthrow["type";(connect;string host;string user;string pass;string connection)];
	};
	should["fail when using connection different than Real or Demo"]{
	 	mustthrow["connection";(connect;host;user;pass;`abcd)];
	 };
 };