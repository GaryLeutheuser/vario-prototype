## TD-001: Tool Defect: Rule 17.3
Tool ID: `misra-c2012-17.3`

### Category
Mandatory.

### Description
This records a tool defect in which cppcheck is unable to properly identify compliance with Rule 17.3, forbidding implicit function declarations.

### Scope
All CMSIS intrinsic macros.

### Rationale
Rule 17.3 forbids implicit function declarations; the call to `__WFI` in `main.c`, for example, is flagged as being an implicit function declaration. This project is compiled with `-Werror -Wimplicit-function-declaration`, which would result in a build time error if the function was truly lacking a declaration. This particular symbol is a macro that expands to assembly, resulting in the false positive from cppcheck.

See `third_party/cmsis_gcc.h`, with an example of `__WFI` below:

```
#define __WFI()         __ASM volatile ("wfi":::"memory")
```

### Accepted By
Gary Leutheuser

### Date
2026-07-31