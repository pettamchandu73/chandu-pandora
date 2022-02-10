#include <stdio.h>
#include "property.h"
char  *  getsetvalue(property  * param  , char *  set , char  * name){
	category  * ctemp ; 
	paramvalue   *   parameter; 
	if(!param->parsed )  return  NULL;
	ctemp = param->phead;
	while(ctemp){
		if(!strcmp(ctemp->category, set)){
			parameter=ctemp->paramhead ;	
			while(parameter){
				if(!strcmp(parameter->name , name)){
					return parameter->value;
				}
				parameter=parameter->next;
			}
		}
		ctemp=ctemp->next;
	}
	return NULL;
}

char *  getvalue(property *param , char  * name  ){
	return param->getsetvalue(param , "general", name);
}

int parse (property  * param){
	FILE * fp=NULL;
	int  headflag=0;
	char buffer[2048], *ptemp  , *ptemp1;
	paramvalue  *  lparam ; 
		fp = fopen(param->configname , "r");
		if(!fp) goto error ;
		param->phead = (category *)malloc(sizeof(category));
		memset(param->phead , 0 , sizeof(category));
		param->pcurrent = param->phead;
		param->pcurrent->paramhead  = (paramvalue *) malloc(sizeof(paramvalue));
		memset(param->pcurrent->paramhead, 0 , sizeof(paramvalue));
		param->pcurrent->parameter = param->pcurrent->paramhead;
		if( param->phead == NULL )  goto error;
		while(!feof(fp)){
			fgets(buffer , sizeof(buffer),fp );				
			//printf( "Line  %s\n", buffer);
			if(buffer[0]== ';')  continue;
			if( buffer[0]=='[' ) {
				//printf( "Inside  mnode %p  paramnode %p\n", param->pcurrent, param->pcurrent->parameter );
				if(headflag)
					param->pcurrent = param->pcurrent->next;
				headflag = 1;	
				ptemp=strchr( &buffer[1], ']');
				if(ptemp) *ptemp=0;
				//printf("Inserting to  node  %s\n",&buffer[1] );
				strncpy(param->pcurrent->category, &buffer[1], sizeof(param->pcurrent->category)-1);
				param->plist = (category *)malloc(sizeof(category));
				memset(param->plist , 0 , sizeof(category));
				param->plist->parameter  = (paramvalue *) malloc(sizeof(paramvalue));
				if(param->plist == NULL) goto error;
				memset(param->plist->parameter , 0 , sizeof(paramvalue));
				param->pcurrent->next = param->plist ; 
				//param->pcurrent->next->parameter = param->plist->parameter;
				param->pcurrent->next->paramhead = param->plist->parameter;
				param->pcurrent->next->parameter = param->plist->parameter;
				continue;
			}
			ptemp=strchr(buffer ,'=');
			if(ptemp){
				lparam  = (paramvalue *) malloc(sizeof(paramvalue));
				if(lparam == NULL) goto error;
				*ptemp = 0 ;
				ptemp++;
				while(*ptemp==0x20){
					*ptemp=0;
					ptemp++;
				}
				ptemp1=strchr(buffer , 0x20);
				if(ptemp1) *ptemp1=0;
				ptemp1 = strchr( ptemp, 0x0d);
				if(ptemp1) *ptemp1 = 0 ;
				ptemp1 = strchr( ptemp, 0x0a);
				if(ptemp1) *ptemp1 = 0 ;
				if(!strlen(param->pcurrent->category))	
					strncpy(param->pcurrent->category , "general", sizeof(param->pcurrent->category)-1);	
				strncpy(param->pcurrent->parameter->name, buffer, sizeof(param->pcurrent->parameter->name)-1);
				strncpy(param->pcurrent->parameter->value, ptemp, sizeof(param->pcurrent->parameter->value)-1);
				param->pcurrent->parameter->next = lparam;
				param->pcurrent->parameter=param->pcurrent->parameter->next;
			}
		
			
		}
		param->parsed = 1;	
	return  0;
error:
	if(fp) fclose(fp);
	return -1;

}
int  destroy( property * param){
	category  * temp ; 
	paramvalue  * parameter , *  prm; 
	while(1){
		temp = param->phead ;
		if(!temp) break;
		param->phead = param->phead->next;
		parameter = temp->paramhead;
		//prm = parameter ;
		while(parameter){
			prm = parameter ;
			parameter = parameter->next;				
			if(parameter==NULL) break;
			free(prm);	
		}
		free(parameter);
	}
}
int init_property(char * filename  , property * param  ){
	strncpy(param->configname,filename, sizeof(param->configname)-1);
	param->parsed = 0 ;
	param->parseproperty = parse; 
	param->getvalue =  getvalue; 
	param->getsetvalue =  getsetvalue; 
	param->destroy =  destroy; 
	return 0 ; 	
			
}
