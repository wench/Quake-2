// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the REFGLGAMMA_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// REFGLGAMMA_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef REFGLGAMMA_EXPORTS
#define REFGLGAMMA_API __declspec(dllexport)
#else
#define REFGLGAMMA_API __declspec(dllimport)
#endif

// This class is exported from the dll
class REFGLGAMMA_API Crefglgamma {
public:
	Crefglgamma(void);
	// TODO: add your methods here.
};

extern REFGLGAMMA_API int nrefglgamma;

REFGLGAMMA_API int fnrefglgamma(void);
