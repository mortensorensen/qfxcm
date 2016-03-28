.tst.desc["QFXCM"]{
	should["fail when using connection different than Real or Demo"]{
	 	mustthrow["connection";(connect;host;user;pass;`abcd)];
	 };
 };