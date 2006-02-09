// $Id$
//
//    File: DBCALMCResponse.h
// Created: Thu Nov 17 09:56:05 CST 2005
// Creator: gluexuser (on Linux hydra.phys.uregina.ca 2.4.20-8smp i686)
//

#ifndef _DBCALMCResponse_
#define _DBCALMCResponse_

#include "DObject.h"
#include "DFactory.h"

class DBCALMCResponse:public DObject{
	public:
		HDCLASSDEF(DBCALMCResponse);
		
		int module;
		int layer;
		int sector;
		int end; /// 0:UPSTREAM(A) 1:DOWNSTREAM(B)
		float E;
		float t;
		
};

#endif // _DBCALMCResponse_

