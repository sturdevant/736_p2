mytest: mytest.C
	g++ -o mytest mytest.C -Iutil -Inodes nodes/Cell.C nodes/LeafNode.C nodes/Request.C nodes/Response.C nodes/Cluster.C nodes/Point.C nodes/InternalNode.C
