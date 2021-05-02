#pragma once

#ifndef _KSN_INSTRUCTION_SET_X86_64_HPP_
#define _KSN_INSTRUCTION_SET_X86_64_HPP_



#include <ksn/ksn.hpp>



_KSN_BEGIN


_KSN_DETAIL_BEGIN

struct __ksn_x86_64_features
{

	//Function 1, edx

	bool fpu : 1; //Floating Point Unit On-Chip. The processor contains an x87 FPU.
	bool vme : 1; //Virtual 8086 Mode Enhancements. Virtual 8086 mode enhancements, including CR4.VME for controlling the feature, CR4.PVI for protected mode virtual interrupts, software interrupt indirection, expansion of the TSS with the software indirection bitmap, and EFLAGS.VIF and EFLAGS.VIP flags. 
	bool de : 1; //Debugging Extensions. Support for I/O breakpoints, including CR4.DE for controlling the feature, and optional trapping of accesses to DR4 and DR5. 
	bool pse : 1; //Page Size Extension. Large pages of size 4 MByte are supported, including CR4.PSE for controlling the feature, the defined dirty bit in PDE (Page Directory Entries), optional reserved bit trapping in CR3, PDEs, and PTEs. 
	bool tsc : 1; //Time Stamp Counter. The RDTSC instruction is supported, including CR4.TSD for controlling privilege.
	bool msr : 1; //Model Specific Registers RDMSR and WRMSR Instructions. The RDMSR and WRMSR instructions are supported. Some of the MSRs are implementation dependent.
	bool pae : 1; //Physical Address Extension.Physical addresses greater than 32 bits are supported: extended page table entry formats, an extra level in the page translation tables is defined, 2-MByte pages are supported instead of 4 Mbyte pages if PAE bit is 1. 
	bool mce : 1; //Machine Check Exception. Exception 18 is defined for Machine Checks, including CR4.MCE for controlling the feature. This feature does not define the model-specific implementations of machine-check error logging, reporting, and processor shutdowns. Machine Check exception handlers may have to depend on processor version to do model specific processing of the exception, or test for the presence of the Machine Check feature.
	bool cmpxchg8 : 1; //CMPXCHG8B Instruction. The compare-and-exchange 8 bytes (64 bits) instruction is supported (implicitly locked and atomic).
	bool apic : 1; //APIC On-Chip. The processor contains an Advanced Programmable Interrupt Controller (APIC), responding to memory mapped commands in the physical address range FFFE0000H to FFFE0FFFH (by default - some processors permit the APIC to be relocated). 
	bool _reserved_leaf1_edx_bit10 : 1;
	bool sep : 1; //SYSENTER and SYSEXIT Instructions. The SYSENTER and SYSEXIT and associated MSRs are supported. 
	bool mtrr : 1; //Memory Type Range Registers. MTRRs are supported. The MTRRcap MSR contains feature bits that describe what memory types are supported, how many variable MTRRs are supported, and whether fixed MTRRs are supported.
	bool pge : 1; //Page Global Bit. The global bit is supported in paging-structure entries that map a page, indicating TLB entries that are common to different processes and need not be flushed. The CR4.PGE bit controls this feature. 
	bool mca : 1; //Machine Check Architecture. A value of 1 indicates the Machine Check Architecture of reporting machine errors is supported. The MCG_CAP MSR contains feature bits describing how many banks of error reporting MSRs are supported. 
	bool cmov : 1; //Conditional Move Instructions. The conditional move instruction CMOV is supported. In addition, if x87 FPU is present as indicated by the CPUID.FPU feature bit, then the FCOMI and FCMOV instructions are supported 
	bool pat : 1; //Page Attribute Table. Page Attribute Table is supported. This feature augments the Memory Type Range Registers (MTRRs), allowing an operating system to specify attributes of memory accessed through a linear address on a 4KB granularity.
	bool pse36 : 1; //36-Bit Page Size Extension. 4-MByte pages addressing physical memory beyond 4 GBytes are supported with 32-bit paging. This feature indicates that upper bits of the physical address of a 4-MByte page are encoded in bits 20:13 of the page-directory entry. Such physical addresses are limited by MAXPHYADDR and may be up to 40 bits in size.
	bool psn : 1; //Processor Serial Number. The processor supports the 96-bit processor identification number feature and the feature is enabled.
	bool clfsh : 1; //CLFLUSH Instruction. CLFLUSH Instruction is supported.
	bool _reserved_leaf1_edx_bit20 : 1;
	bool ds : 1; //Debug Store. The processor supports the ability to write debug information into a memory resident buffer. This feature is used by the branch trace store (BTS) and processor event-based sampling (PEBS) facilities
	bool acpi : 1; //Thermal Monitor and Software Controlled Clock Facilities. The processor implements internal MSRs that allow processor temperature to be monitored and processor performance to be modulated in predefined duty cycles under software control.
	bool mmx : 1; //Intel MMX Technology. The processor supports the Intel MMX technology.
	bool fxsr : 1; //FXSAVE and FXRSTOR Instructions. The FXSAVE and FXRSTOR instructions are supported for fast save and restore of the floating point context. Presence of this bit also indicates that CR4.OSFXSR is available for an operating system to indicate that it supports the FXSAVE and FXRSTOR instructions.
	bool sse : 1; //SSE. The processor supports the SSE extensions
	bool sse2 : 1; //SSE2. The processor supports the SSE2 extensions
	bool ss : 1; //Self Snoop. The processor supports the management of conflicting memory types by performing a snoop of its own cache structure for transactions issued to the bus.
	bool htt : 1; //Max APIC IDs reserved field is Valid. A value of 0 for HTT indicates there is only a single logical processor in the package and software should assume only a single APIC ID is reserved. A value of 1 for HTT indicates the value in CPUID.1.EBX[23:16] (the Maximum number of addressable IDs for logical processors in this package) is valid for the package.
	bool tm : 1; //Thermal Monitor. The processor implements the thermal monitor automatic thermal control circuitry (TCC).
	bool ia64 : 1; //
	bool pbe : 1; //Pending Break Enable. The processor supports the use of the FERR#/PBE# pin when the processor is in the stop-clock state (STPCLK# is asserted) to signal the processor that an interrupt is pending and that the processor should return to normal operation to handle the interrupt. Bit 10 (PBE enable) in the IA32_MISC_ENABLE MSR enables this capability.



	//Function 1, ecx

	bool sse3 : 1; //Streaming SIMD Extensions 3 (SSE3). A value of 1 indicates the processor supports this technology.
	bool pclmulqdq : 1; //PCLMULQDQ. A value of 1 indicates the processor supports the PCLMULQDQ instruction.
	bool dtes64 : 1; //64-bit DS Area. A value of 1 indicates the processor supports DS area using 64-bit layout
	bool monitor : 1; //MONITOR/MWAIT. A value of 1 indicates the processor supports this feature. 
	bool ds_cpl : 1; //CPL Qualified Debug Store. A value of 1 indicates the processor supports the extensions to the Debug Store feature to allow for branch message storage qualified by CPL
	bool vmx : 1; //Virtual Machine Extensions. A value of 1 indicates that the processor supports this technology.
	bool smx : 1; //Safer Mode Extensions. A value of 1 indicates that the processor supports this technology. See Chapter 6, “Safer Mode Extensions Reference”
	bool est : 1; //Enhanced Intel SpeedStep® technology. A value of 1 indicates that the processor supports this technology.
	bool tm2 : 1; //Thermal Monitor 2. A value of 1 indicates whether the processor supports this technology. 
	bool ssse3 : 1; //A value of 1 indicates the presence of the Supplemental Streaming SIMD Extensions 3 (SSSE3). A value of 0 indicates the instruction extensions are not present in the processor
	bool cntx_id : 1; //L1 Context ID. A value of 1 indicates the L1 data cache mode can be set to either adaptive mode or shared mode. A value of 0 indicates this feature is not supported. See definition of the IA32_MISC_ENABLE MSR Bit 24 (L1 Data Cache Context Mode) for details
	bool sdbg : 1; //A value of 1 indicates the processor supports IA32_DEBUG_INTERFACE MSR for silicon debug.
	bool fma : 1; //A value of 1 indicates the processor supports FMA extensions using YMM state.
	bool cmpxchg16b : 1; //CMPXCHG16B Available. A value of 1 indicates that the feature is available. See the “CMPXCHG8B/CMPXCHG16B—Compare and Exchange Bytes” section in this chapter for a description.
	bool xtpr : 1; //xTPR Update Control. A value of 1 indicates that the processor supports changing IA32_MISC_ENABLE[bit 23].
	bool pdcm : 1; //Perfmon and Debug Capability: A value of 1 indicates the processor supports the performance and debug feature indication MSR IA32_PERF_CAPABILITIES
	bool _reserved_leaf1_ecx_bit16 : 1;
	bool pcid : 1; //Process-context identifiers. A value of 1 indicates that the processor supports PCIDs and that software may set CR4.PCIDE to 1.
	bool dca : 1; //A value of 1 indicates the processor supports the ability to prefetch data from a memory mapped device.
	bool sse4_1 : 1; //A value of 1 indicates that the processor supports SSE4.1.
	bool sse4_2 : 1; //A value of 1 indicates that the processor supports SSE4.2.
	bool x2apic : 1; //A value of 1 indicates that the processor supports x2APIC feature.
	bool movbe : 1; //A value of 1 indicates that the processor supports MOVBE instruction.
	bool popcnt : 1; //A value of 1 indicates that the processor supports the POPCNT instruction.
	bool tsc_deadline : 1; //A value of 1 indicates that the processor’s local APIC timer supports one-shot operation using a TSC deadline value.
	bool aesni : 1; //A value of 1 indicates that the processor supports the AESNI instruction extensions.
	bool xsave : 1; //A value of 1 indicates that the processor supports the XSAVE/XRSTOR processor extended states feature, the XSETBV/XGETBV instructions, and XCR0.
	bool osxsave : 1; //A value of 1 indicates that the OS has set CR4.OSXSAVE[bit 18] to enable XSETBV/XGETBV instructions to access XCR0 and to support processor extended state management using XSAVE/XRSTOR.
	bool avx : 1; //A value of 1 indicates the processor supports the AVX instruction extensions.
	bool f16c : 1; //A value of 1 indicates that processor supports 16-bit floating-point conversion instructions.
	bool rdrnd : 1; //A value of 1 indicates that processor supports RDRAND instruction.
	bool hypervisor : 1; //Hypervisor present (always zero on physical CPUs)



	//Function 7.0, ebx

	bool fsgsbase : 1;
	bool ia32_tsc_adjust : 1;
	bool sgx : 1;
	bool bmi1 : 1;
	bool hle : 1;
	bool avx2 : 1;
	bool fdp_excptn_only : 1;
	bool smep : 1;
	bool bmi2 : 1;
	bool erms : 1;
	bool invpcid : 1;
	bool rtm : 1;
	bool pqm : 1;
	bool fpu_cs_ds_deprecated : 1;
	bool mpx : 1;
	bool pqe : 1;
	bool avx512_f : 1;
	bool avx512_dq : 1;
	bool rdseed : 1;
	bool adx : 1;
	bool smap : 1;
	bool avx512_ifma : 1;
	bool pcommit : 1;
	bool clflushopt : 1;
	bool clwb : 1;
	bool intel_pt : 1;
	bool avx512_pf : 1;
	bool avx512_er : 1;
	bool avx512_cd : 1;
	bool sha : 1;
	bool avx512_bw : 1;
	bool avx512_vl : 1;



	//Function 7.0, ecx

	bool prefetchwt1 : 1;
	bool avx512_vbmi : 1;
	bool umip : 1;
	bool pku : 1;
	bool ospke : 1;
	bool waitpkg : 1;
	bool avx512_vbmi2 : 1;
	bool cet_ss : 1;
	bool gfni : 1;
	bool vaes : 1;
	bool vpclmulqdq : 1;
	bool avx512_vnni : 1;
	bool avx512_bitalg : 1;
	bool _reserved_leaf7_sub0_ecx_bit13 : 1;
	bool avx512_vpopcntdq : 1;
	bool _reserved_leaf7_sub0_ecx_bit15 : 1;
	bool five_level_paging : 1;
	uint8_t mawau : 5;
	bool rdpid : 1;
	bool _reserved_leaf7_sub0_ecx_bit23 : 1;
	bool _reserved_leaf7_sub0_ecx_bit24 : 1;
	bool cldemote : 1;
	bool _reserved_leaf7_sub0_ecx_bit26 : 1;
	bool movdir : 1;
	bool movdir64b : 1;
	bool _reserved_leaf7_sub0_ecx_bit29 : 1;
	bool sgx_lc : 1;
	bool _reserved_leaf7_sub0_ecx_bit31 : 1;



	//Function 7.0, edx

	bool _reserved_leaf7_sub0_edx_bit0 : 1;
	bool _reserved_leaf7_sub0_edx_bit1 : 1;
	bool avx512_4vnniw : 1;
	bool avx512_4fmaps : 1;
	bool fsrm : 1;
	bool _reserved_leaf7_sub0_edx_bit5 : 1;
	bool _reserved_leaf7_sub0_edx_bit6 : 1;
	bool _reserved_leaf7_sub0_edx_bit7 : 1;
	bool _reserved_leaf7_sub0_edx_bit8 : 1;
	bool _reserved_leaf7_sub0_edx_bit9 : 1;
	bool _reserved_leaf7_sub0_edx_bit10 : 1;
	bool _reserved_leaf7_sub0_edx_bit11 : 1;
	bool _reserved_leaf7_sub0_edx_bit12 : 1;
	bool _reserved_leaf7_sub0_edx_bit13 : 1;
	bool _reserved_leaf7_sub0_edx_bit14 : 1;
	bool hybrid : 1;
	bool _reserved_leaf7_sub0_edx_bit16 : 1;
	bool _reserved_leaf7_sub0_edx_bit17 : 1;
	bool _reserved_leaf7_sub0_edx_bit18 : 1;
	bool _reserved_leaf7_sub0_edx_bit19 : 1;
	bool cet_bit : 1;
	bool _reserved_leaf7_sub0_edx_bit21 : 1;
	bool _reserved_leaf7_sub0_edx_bit22 : 1;
	bool _reserved_leaf7_sub0_edx_bit23 : 1;
	bool _reserved_leaf7_sub0_edx_bit24 : 1;
	bool _reserved_leaf7_sub0_edx_bit25 : 1;
	bool spec_ctrl : 1;
	bool stibp : 1;
	bool l1d_flush : 1;
	bool ia32_arch_capabilities : 1;
	bool ia32_core_capabilities : 1;
	bool ssbd : 1;



	//Function 7.1, eax

	bool _reserved_leaf7_sub1_eax_bits0_4 : 5;
	bool avx512_bf16 : 1;
	bool _reserved_leaf7_sub1_eax_bit6 : 1;
	bool _reserved_leaf7_sub1_eax_bit7 : 1;
	bool _reserved_leaf7_sub1_eax_bits8_15: 8;
	bool _reserved_leaf7_sub1_eax_bits16_23: 8;
	bool _reserved_leaf7_sub1_eax_bits24_31: 8;



	//Function 13.1, eax

	bool xsaveopt : 1;
	bool xsavec : 1;
	bool xgetbv_ecx1 : 1;
	bool xsaves : 1;
	bool _reserved_leaf13_sub1_eax_bits4_7 : 4;
	bool _reserved_leaf13_sub1_eax_bits8_15: 8;
	bool _reserved_leaf13_sub1_eax_bits16_23: 8;
	bool _reserved_leaf13_sub1_eax_bits24_31: 8;



	//Function 14, eax

	bool _reserved_leaf14_sub0_eax_bits0_3 : 4;
	bool ptwrite : 1;
	bool _reserved_leaf14_sub0_eax_bits5_7 : 3;
	bool _reserved_leaf14_sub0_eax_bits8_15: 8;
	bool _reserved_leaf14_sub0_eax_bits16_23: 8;
	bool _reserved_leaf14_sub0_eax_bits24_31: 8;



	//leaf 0x80000001, ecx
	//named after amd's specs

	bool lahf_lm : 1; //LAHF/SAHF in long mode
	bool cmp_legacy : 1;//Hyperthreading not valid
	bool svm : 1; //Secure Virtual Machine;
	bool extapic : 1; //Extended APIC space
	bool cr8_legacy : 1; //CR8 in 32 - bit mode;
	bool abm_lzcnt_popcnt : 1; //Advanced bit manipulation (lzcnt and popcnt)
	bool sse4a : 1; //SSE4a
	bool misalignsse : 1; //Misaligned SSE mode
	bool prefetchw : 1; //PREFETCH and PREFETCHW instructions 
	bool osvw : 1; //OS Visible Workaround 
	bool ibs : 1; //Instruction Based Sampling
	bool xop : 1; //XOP instruction set
	bool skinit : 1; //SKINIT/STGI instructions 
	bool wdt : 1; //Watchdog timer
	bool _reserved_leaf1e_ecx_bit14 : 1;
	bool lwp : 1; //Light Weight Profiling
	bool fma4 : 1; //4 operands fused multiply-add
	bool tce : 1; //Translation Cache Extension 
	bool _reserved_leaf1e_ecx_bit18 : 1;
	bool nodeid_msr : 1; //NodeID MSR 
	bool _reserved_leaf1e_ecx_bit20 : 1;
	bool tbm : 1; //Trailing Bit Manipulation
	bool topoext : 1; //Topology Extensions
	bool perfctr_core : 1; //Core performance counter extensions 
	bool perfctr_nb : 1; //NB performance counter extensions
	bool _reserved_leaf1e_ecx_bit25 : 1;
	bool dbx : 1; //Data breakpoint extensions 
	bool perftss : 1; //Performance TSC 
	bool pcx_l2i : 1; //L2I perf counter extensions 
	bool _reserved_leaf1e_ecx_bit29_31 : 3;


	//leaf 0x80000001, edx

	bool fpu_leaf1e : 1;
	bool vme_leaf1e : 1;
	bool de_leaf1e : 1;
	bool pse_leaf1e : 1;
	bool tsc_leaf1e : 1;
	bool msr_leaf1e : 1;
	bool pae_leaf1e : 1;
	bool mce_leaf1e : 1;
	bool cx8 : 1;
	bool apic_leaf1e : 1;
	bool _reserved_leaf1e_edx_bit10 : 1;
	bool syscall : 1;
	bool mtrr_leaf1e : 1;
	bool pge_leaf1e : 1;
	bool mca_leaf1e : 1;
	bool cmov_leaf1e : 1;
	bool pat_leaf1e : 1;
	bool pse36_leaf1e : 1;
	bool _reserved_leaf1e_edx_bit18 : 1;
	bool mp : 1;
	bool nx : 1;
	bool _reserved_leaf1e_edx_bit21 : 1;
	bool mmxext : 1;
	bool mmx_leaf1e : 1;
	bool fxsr_leaf1e : 1;
	bool fxsr_opt : 1;
	bool pdpe1gb : 1;
	bool rdtscp : 1;
	bool _reserved_leaf1e_edx_bit28 : 1;
	bool lm : 1;
	bool _3dnowext : 1;
	bool _3dnow : 1;
};

static_assert(sizeof(__ksn_x86_64_features) == 40, "");

_KSN_DETAIL_END


extern const detail::__ksn_x86_64_features* x86_features;

bool cpuid(int leaf, uint32_t& eax, uint32_t& ebx, uint32_t& ecx, uint32_t& edx, int subleaf = 0) noexcept;

bool cpuid(int leaf, uint32_t* registers, int subleaf = 0) noexcept;

bool init_feature_struct(detail::__ksn_x86_64_features* p) noexcept;

_KSN_END

#endif //!_KSN_INSTRUCTION_SET_X86_64_HPP_
