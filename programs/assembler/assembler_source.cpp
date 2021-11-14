/*

; execution starts here
jmp (start)

; OS variables

.console. = 67

; small variables (less than one operand)
.assemblyCodeStartPoint. = 0
.assemblyCodeEndPoint. = 0
.outputCodeStartPoint. = 0
.outputCodeEndPoint. = 0
.wordTypeExpected. = 0
.curLine. = 1
.codePosAtLastNewLine. = 0
.charCache_CharsStored. = 0
.labelDefCache_Stored. = 0
.labelRefCache_Stored. = 0
.outputTempStore. = 0
.outputCode_InstructionsStored. = 0
.wordIdentified. = 0

.maxWordLength. = 20
.labelCacheSize. = 256

; large variables (greater than one operand)
.assemblyCode.
.outputCode.
.labelDefCache.
.labelRefCache.
.labelRefCache_matched.

; temporary use variables (reset often)

; Iterate through each character in the assembly code
; load initial values for variables
.start. = LAA (assemblyCodeStartPoint)
ADD 0 ; clear
STR (i0)
LAA (assemblyCodeEndPoint)
STR (i1)
; check if i0 <= i1
LDA .i1. = 0
SBA (i0)
JIE (for_end0)

LDA (console)
SOT (i0)

; increment position in the assembly code
LDA .i0. = 0
ADD 1
STR (i0)
JMP (i1)
.for_end0. = 0

.end. 

*/