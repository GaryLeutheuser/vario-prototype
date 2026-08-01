#!/bin/bash
cppcheck --project=build/Debug/compile_commands.json --enable=warning,style,performance,portability,information --platform=arm32-wchar_t4 --inline-suppr --suppressions-list=tools/suppressions.txt --include=tools/cppcheck-defines.h --checkers-report=tools/check_report.txt --addon=tools/misra.json
