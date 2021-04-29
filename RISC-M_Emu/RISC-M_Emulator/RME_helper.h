#ifndef RISC_M_EMULATOR_RME_HELPER_H_
#define RISC_M_EMULATOR_RME_HELPER_H_

#ifdef MAKE_DLL
	#define RME_fprintf(arg1,arg2,...) (void)0
#else
	#define RME_fprintf(arg1,arg2,...) fprintf(arg1,arg2,##__VA_ARGS__)
#endif

#endif /* RISC_M_EMULATOR_RME_HELPER_H_ */
