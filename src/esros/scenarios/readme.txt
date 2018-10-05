invoker
1001	saprelease 12; sapbind 12; invoke; (result);    saprelease
1002	saprelease 12; sapbind 12; invoke; (error);     saprelease
1003	saprelease 12; sapbind 12; invoke; (failure 1); saprelease
1004	saprelease 12; sapbind 12; invoke; (failure 2); saprelease
1005	saprelease 12; sapbind 12; invoke; (None);      saprelease

invokations with invalid parameters
2001	invoke; result (no sapbind)
2002	invoke; result (invalid SAP)
2003	invoke; result (invalid IP)
2004	invoke; result (invalid port)
2005	invoke; result (invalid op code)
2006	invoke; result (invalid/unknown encoding)

Multiple invokations
3001	[invoke result] [invoke result]
3002	[invoke result] [invoke error]
3003	[invoke result] [invoke error] [invoke result] [invoke error] 
	[invoke error] [invoke error] [invoke result] [invoke error]

Stress test
4001 	operation with large data (100 bytes)
4002 	operation with large data (1K bytes)
4003 	50 operations
4004 	100 operations
4005 	operation with 100 bytes data
4006 	operation with 1K bytes data
4007 	operation with large data (2K bytes)
4008 	2000 operations
4009 	operation with 3K bytes data

