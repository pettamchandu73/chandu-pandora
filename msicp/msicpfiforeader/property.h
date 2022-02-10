#ifndef  __PROPERTY__
	#define  __PROPERTY__ 
	#include <string.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <stdlib.h>	
	typedef struct paramval paramvalue;
	typedef struct sets  category;   
	struct sets {
		char  category[128];
		paramvalue  *paramhead;
		paramvalue  * parameter ;
		category  *  next;
	};
	struct paramval{
		char name[128];
		char value[1024];
		paramvalue  * next ; 
	};
	typedef  struct  config property;
	struct  config {
		char configname[256];
	//	paramvalue * phead , *pcurrent, *plist; 
		category * phead , *pcurrent, *plist;
		int  parsed; 
		int	  (*parseproperty)(property * param ); 
		char *(*getvalue)( property * param , char * name );
		char *(*getsetvalue)(property * param  , char  * set , char * name );
		int   (*destroy)( property * param );
	}; 
	int init_property(char * , property *  );
#endif
