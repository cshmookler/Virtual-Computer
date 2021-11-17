/*

; execution starts here
jmp (start)

; small variables (one operand)
.wordTypeExpected. = 0
.charsReadInWord. = 0
.labelDefCache_stored. = 0
.labelRefCache_stored. = 0
.curLine. = 1
.codePosAtLastNewLine. = 0
.outputTempStore. = 0
.outputCode_instructionsStored. = 0
.wordIdentified. = 0

; large variables (greater than one operand)
.assemblyCode_start. = 0 0 0 0 0 0 0 0 0 .assemblyCode_end. = 0
.outputCode_start. = 0 0 0 0 0 0 0 0 0 .outputCode_end. = 0
.labelDefCache_start. = 0 0 0 0 0 0 0 0 0 0 0 .labelDefCache_end. = 0
.labelRefCache_start. = 0 0 0 0 0 0 0 0 0 0 0 .labelRefCache_end. = 0
.labelRefCache_matched_start. = 0 0 0 .labelRefCache_matched_end. = 0

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

; inside the loop


; increment position in the assembly code
LDA .i0. = 0
ADD 1
STR (i0)
JMP (i1)
.for_end0. = 0

.end. 

*/
