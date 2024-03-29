/*
  ===========================================================================
	MacHeadersMach-O.h			�2000 Metrowerks Inc. All rights reserved.
  ===========================================================================

	Processor independant interface to the MacHeadersMach-O files
*/

#if __MWERKS__ < 0x3000

#ifdef __cplusplus
	#include <MacHeadersMach-O++>
#else
	#include <MacHeadersMach-O>
#endif

#else

/*
  ===========================================================================
	MSL MacHeadersMach-O.h	� 2000-2002 Metrowerks Corporation.  All rights reserved.
  ===========================================================================

	Processor independant interface to the MSLMacHeadersMach-O files
*/

#if __mwlinker__
	#ifdef __cplusplus
		#include <MSLMacHeadersMacOSX++>
	#else
		#include <MSLMacHeadersMacOSX>
	#endif
#else
	#ifdef __cplusplus
		#include <MSLMacHeadersMach-O++>
	#else
		#include <MSLMacHeadersMach-O>
	#endif
#endif

#endif