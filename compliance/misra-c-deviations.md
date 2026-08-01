# MISRA-C Deviations
## D-001: Rule 11.4
Tool ID: `misra-c2012-11.4`
### Category
Required.

### Description
This deviation is for a suppression of compliance issues against conversion of pointers to arithmetic types - this is fundamental to memory-mapped hardware access.

### Scope
The scope of the suppression is for memory-mapped peripheral access in the project's Hardware Abstraction Layer (HAL) (`Src/hal/`).

### Rationale
Hardware memory accesses are done through vendor-provided CMSIS header files for the device. These are located in `third_party/`. Addresses that are accessed are verified against the device memory map (RM0090 Reference Manual). There are no hand computed addresses accessed; it is a static listing that is cross-checked. This provides a best-effort mitigation against the risk of accessing a misaligned pointer.

### Accepted By
Gary Leutheuser

### Date
2026-07-31

## D-002: Rule 8.7
Tool ID: `misra-c2012-8.7`
### Category
Advisory.

### Description
This deviation is for Interrupt Service Routines (ISRs) being in violation of limiting visibility of functions which are not referenced in more than one location.

### Scope
All ISR functions as listed in `startup_stm32f407xx.S`.

### Rationale
ISRs are special functions, triggered only by interrupt hardware on the device, and not generally called directly. These interrupt handlers are referenced in the file `startup_stm32f407xx.S` and thus require external linkage; a search against the codebase for each ISR name can be used to confirm that these functions are not called directly by any other part of the code.

### Accepted By
Gary Leutheuser

### Date
2026-07-31