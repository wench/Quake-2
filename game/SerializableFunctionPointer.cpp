#include "SerializableFunctionPointer.h"
#include "q_shared.h"
 ISerializableFunctionPointer *ISerializableFunctionPointer::list=nullptr;

 ISerializableFunctionPointer* ISerializableFunctionPointer::GetInterface(const char* name)
 {
	 for (ISerializableFunctionPointer* cur = list; cur; cur = cur->next)
	 {
		 if (!strcmp(cur->name, name))
			 return cur;

	 }
	 return nullptr;
 } 
 ISerializableFunctionPointer* ISerializableFunctionPointer::GetfromFunction(void* func)
 {
	 for (ISerializableFunctionPointer* cur = list; cur; cur = cur->next)
	 {
		 if (cur->get_fpointer()==func)
			 return cur;

	 }
	 return nullptr;
 }