/*

Initially, A three character op-code is expected. Then, after any number of spacers, an operand for the previous op-code is expected. An operand can consist of one of the following: a binary integer (indicated by the prefix "b"), a decimal integer (no prefix), or a label (indicated by the prefix "." and followed by the name of the label). After the operand, any amount of spacers are permitted before the next instruction. The next instruction may be on the same line as the previous instruction. If the ";" (semi-colon) character is encountered, all subsequent characters are ignored until the "\n" (new line) character is encountered. Op-code names, label names, the "b" prefix before binary integers, the "o" prefix before octal integers, and the "h" prefix before hexadecimal integers are not case-sensitive. If any of the above rules are violated, throw an error and stop the assembly.

    ; Label Definition
    ADD .saveNum. ; "LDA" is the instruction, ".saveNum." is the label declaration, and "0" is the default value.

    ; Label Reference (Pointer)
    JMP (saveNum) ; "JMP" is the instruction, "(saveNum)" refers to the memory location of ".saveNum."

Label references cannot set default values. There can only be one label definition for each label name. Similar label names with different capitalization are interpreted as the same label name.

*/

/*

    reserved characters (only for specific purposes):

        - '.' (starts/ends a label declaration)
        - '(' (starts a label reference)
        - ')' (ends a label reference)
        - ';' (starts a comment)
        - '=' (assigns a default value to a label declaration)
    
    controlled characters (only for use in instruction names, label declaration names, label reference names, and values):

        - characters 'a' -> 'z' (NOTE: lowercase characters are interpreted as uppercase characters!)
        - characters 'A' -> 'Z'
        - characters '0' -> '0'
        - '_'
    
    special function characters:

        - '\n' (ends a comment)

*/

#include <iostream>
#include <string.h>

// Error reporting
void throwError(int code, int curLine, int codePos, int codePosAtLastNewLine)
{
    if(code == 16 || code == 17)
    {
        std::cout << "Syntax error (location unknown)" << std::endl;
    }
    else
    {
        std::cout << "Syntax error at line " << curLine << ", column " << codePos - codePosAtLastNewLine << std::endl;
    }

    switch(code)
    {
        case 1:
            throw std::runtime_error("comments cannot be used inside of instructions (error code: 1)");
            break;
        case 2:
            throw std::runtime_error("words missing at the end of the file (error code: 2)");
            break;
        case 3:
            throw std::runtime_error("unexpected character (error code: 3)");
            break;
        case 4:
            throw std::runtime_error("unknown operand type identifier (error code: 4)");
            break;
        case 5:
            throw std::runtime_error("unknown instruction (error code: 5)");
            break;
        case 6:
            throw std::runtime_error("incomplete instruction (error code: 6)");
            break;
        case 7:
            throw std::runtime_error("word is too long (exceeds 25 characters) (error code: 7)");
            break;
        case 8:
            throw std::runtime_error("provided value is too large (exceeds 4095) (error code: 8)");
            break;
        case 9:
            throw std::runtime_error("spacers cannot be used in either instruction names or operands (error code: 9)");
            break;
        case 10:
            throw std::runtime_error("periods can only be used to begin or end label declarations (error code: 10)");
            break;
        case 11:
            throw std::runtime_error("equal signs can only be used to set default values for label declarations (error code: 11)");
            break;
        case 12:
            throw std::runtime_error("an equal sign was expected but was not encountered (error code: 12)");
            break;
        case 13:
            throw std::runtime_error("open parentheses can only be used to begin label references (error code: 13)");
            break;
        case 14:
            throw std::runtime_error("close parentheses can only be used to end label references (error code: 14)");
            break;
        case 15:
            throw std::runtime_error("instruction output value is too large (exceeds 65535) (error code: 15)");
            break;
        case 16:
            throw std::runtime_error("duplicate label declaration (error code: 16)");
            break;
        case 17:
            throw std::runtime_error("label reference without a matching label declaration (error code: 17)");
            break;
    }
}

int main()
{
    const char * assemblyCode = "JMP (start) .console. = 67\n.assemblyCodeStartPoint. = 15\n.assemblyCodeEndPoint. = 17\n; execution starts here\n\n; Iterate through each character in the assembly code\n; load initial values for variables\n.start. = LAA (assemblyCodeStartPoint)\nADD 0 ; clear\nSTR (i0)\nLAA (assemblyCodeEndPoint)\nSTR (i1)\n; check if i0 <= i1\nLDA .i1. = 0\nSBA (i0)\nJIE (for_end0)\n\nLDA (console)\nSOT (i0)\n\n; increment position in the assembly code\nLDA .i0. = 0\nADD 1\nSTR (i0)\nJMP (i1)\n.for_end0. = 0 ";

    int assemblyCodeStartPoint = 0,
        assemblyCodeEndPoint = strlen(assemblyCode),
        outputCodeStartPoint = 0,
        outputCodeEndPoint = 1,
        wordTypeExpected = 0, // 0 = operation expected
                              // 1 = operand expected (operand type identifier expected)
                              // 2 = label declaration expected
                              // 3 = label reference expected
                              // 4 = binary (sixteen digits)
                              // 5 = decimal (default) (four digits, overflow if number is greater than 4095)
                              // 6 = equal sign expected
                              // 7 = instruction or operand expected (operand type identifier expected), but not a label declaration
                              // 8 = operand expected (operand type identifier expected), but not a label declaration
        maxWordLength = 25,
        curLine = 1, // for reporting the position of errors in the source code
        codePosAtLastNewLine = 0, // also for reporting the position of errors in the source code
        charCache_CharsStored = 0,
        labelDefCache[256][3], // First value is position in output, the second value is position in code, and the third value is length in code
        labelRefCache[256][3], // First value is position in output, the second value is position in code, the third value is length in code, and the fourth value is whether or not it has been chcked
        labelDefCache_Stored = 0,
        labelRefCache_Stored = 0,
        outputTempStore,
        outputCode[4096],
        outputCode_InstructionsStored = 0;

        char charCache[maxWordLength];

    bool wordIdentified = false,
         labelRefCache_matched[256] = {0}; // Label references that have been matched with a label declaration are denoted with 'true'

    // assemble to binary (NOT complete)
    for(int codePos = assemblyCodeStartPoint; codePos < assemblyCodeEndPoint; codePos++)
    {
        charCache[charCache_CharsStored] = assemblyCode[codePos];

        switch((int)charCache[charCache_CharsStored])
        {
            // chars "a" -> "z" (Complete)
            case 97: case 98: case 99: case 100: case 101: case 102: case 103: case 104: case 105:
            case 106: case 107: case 108: case 109: case 110: case 111: case 112: case 113: case 114:
            case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122:
                {
                    charCache[charCache_CharsStored] = (char)((int)charCache[charCache_CharsStored] - 32); // This is way simpler in assembly, just decrement charCache[charCache_CharsStored] by 32 (makes it uppercase)
                }
                // no break statement, fallthrough to next case
            
            // chars "A" -> "Z" & "_" (Complete)
            case 65: case 66: case 67: case 68: case 69: case 70: case 71: case 72: case 73:
            case 74: case 75: case 76: case 77: case 78: case 79: case 80: case 81: case 82:
            case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90:
            case 95: // underscore
                {
                    charCache_CharsStored += 1;
                    if(charCache_CharsStored >= maxWordLength)
                    {
                        throwError(7, curLine, codePos, codePosAtLastNewLine);
                    }

                    if(wordTypeExpected == 0 || wordTypeExpected == 7) // for instruction (Complete)
                    {
                        if(charCache_CharsStored == 3)
                        {
                            if(charCache[0] == 'L')
                            {
                                if(charCache[1] == 'D')
                                {
                                    if(charCache[2] == 'A')
                                    {
                                        outputTempStore = 0;
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'A')
                                {
                                    if(charCache[2] == 'A')
                                    {
                                        outputTempStore = 4096; // b0001000000000000
                                        wordIdentified = true;
                                    }
                                }
                            }
                            else if(charCache[0] == 'A')
                            {
                                if(charCache[1] == 'D')
                                {
                                    if(charCache[2] == 'D')
                                    {
                                        outputTempStore = 8192; // b0010000000000000
                                        wordIdentified = true;
                                    }
                                    else if(charCache[2] == 'A')
                                    {
                                        outputTempStore = 16384; // b0100000000000000
                                        wordIdentified = true;
                                    }
                                }
                            }
                            else if(charCache[0] == 'S')
                            {
                                if(charCache[1] == 'B')
                                {
                                    if(charCache[2] == 'D')
                                    {
                                        outputTempStore = 12288; // b0011000000000000
                                        wordIdentified = true;
                                    }
                                    else if(charCache[2] == 'A')
                                    {
                                        outputTempStore = 20480; // b0101000000000000
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'T')
                                {
                                    if(charCache[2] == 'R')
                                    {
                                        outputTempStore = 24576; // b0110000000000000
                                        wordIdentified = true;
                                    }
                                    else if(charCache[2] == 'D')
                                    {
                                        outputTempStore = 28672; // b0111000000000000
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'S')
                                {
                                    if(charCache[2] == 'D')
                                    {
                                        outputTempStore = 32768; // b1000000000000000
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'O')
                                {
                                    if(charCache[2] == 'T')
                                    {
                                        outputTempStore = 61440; // b1111000000000000
                                        wordIdentified = true;
                                    }
                                }
                            }
                            else if(charCache[0] == 'J')
                            {
                                if(charCache[1] == 'M')
                                {
                                    if(charCache[2] == 'P')
                                    {
                                        outputTempStore = 36864; // b1001000000000000
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'I')
                                {
                                    if(charCache[2] == 'Z')
                                    {
                                        outputTempStore = 40960; // b1010000000000000
                                        wordIdentified = true;
                                    }
                                    else if(charCache[2] == 'E')
                                    {
                                        outputTempStore = 45056; // b1011000000000000
                                        wordIdentified = true;
                                    }
                                    else if(charCache[2] == 'I')
                                    {
                                        outputTempStore = 49152; // b1100000000000000
                                        wordIdentified = true;
                                    }
                                }
                                else if(charCache[1] == 'B')
                                {
                                    if(charCache[2] == 'T')
                                    {
                                        outputTempStore = 53248; // b1101000000000000
                                        wordIdentified = true;
                                    }
                                }
                            }
                            else if(charCache[0] == 'G')
                            {
                                if(charCache[1] == 'I')
                                {
                                    if(charCache[2] == 'N')
                                    {
                                        outputTempStore = 57344; // b1110000000000000
                                        wordIdentified = true;
                                    }
                                }
                            }

                            if(wordIdentified == true)
                            {
                                wordIdentified = false;
                                charCache_CharsStored = 0;
                                if(wordTypeExpected == 0)
                                {
                                    wordTypeExpected = 1;
                                }
                                else // wordTypeExpected == 7
                                {
                                    wordTypeExpected = 8;
                                }
                            }
                            else
                            {
                                throwError(5, curLine, codePos, codePosAtLastNewLine);
                            }
                        }
                        else if(charCache_CharsStored == 1)
                        {
                            if(charCache[0] == 'B')
                            {
                                wordTypeExpected = 4;
                                charCache_CharsStored = 0;
                            }
                        }
                    }
                    else if(wordTypeExpected == 1 || wordTypeExpected == 7 || wordTypeExpected == 8) // for operand (Complete)
                    {
                        if(charCache_CharsStored == 1)
                        {
                            charCache_CharsStored -= 1;

                            if(charCache[0] == 'B')
                            {
                                wordTypeExpected = 4;
                            }
                            else
                            {
                                throwError(4, curLine, codePos, codePosAtLastNewLine);
                            }
                        }
                    }
                    else if(wordTypeExpected == 6)
                    {
                        throwError(12, curLine, codePos, codePosAtLastNewLine);
                    }
                }
                break;
            
            // chars 0 -> 9 (Complete)
            case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
                {
                    charCache_CharsStored += 1;
                    if(charCache_CharsStored >= maxWordLength)
                    {
                        throwError(7, curLine, codePos, codePosAtLastNewLine);
                    }

                    if(wordTypeExpected == 6)
                    {
                        throwError(12, curLine, codePos, codePosAtLastNewLine);
                    }
                    else if(wordTypeExpected == 0 || wordTypeExpected == 1 || wordTypeExpected == 7 || wordTypeExpected == 8) // If an operand type identifier is expected (and a number is the first character)
                    {
                        if(charCache_CharsStored == 1)
                        {
                            wordTypeExpected = 5; // Expect a decimal number
                        }
                    }
                }
                break;

            // period (start/end label declaration) (Complete)
            case 46:
                {
                    if(wordTypeExpected == 1 || wordTypeExpected == 0)
                    {
                        if(charCache_CharsStored == 0)
                        {
                            if(wordTypeExpected == 0) // Reset 'outputTempStore' if a label declaration is used without an instruction preceding it
                            {
                                outputTempStore = 0;
                            }

                            labelDefCache[labelDefCache_Stored][0] = outputCode_InstructionsStored; // Store the output position of the label declaration about to be read
                            labelDefCache[labelDefCache_Stored][1] = codePos + 1; // Store the code starting position of the label declaration about to be read
                            wordTypeExpected = 2;
                        }
                        else
                        {
                            throwError(10, curLine, codePos, codePosAtLastNewLine);
                        }
                    }
                    else if(wordTypeExpected == 2)
                    {
                        if(charCache_CharsStored != 0)
                        {
                            labelDefCache[labelDefCache_Stored][2] = codePos - labelDefCache[labelDefCache_Stored][1];
                            labelDefCache_Stored += 1;
                            charCache_CharsStored = 0;
                            wordTypeExpected = 6; // expect an equal sign
                        }
                        else
                        {
                            throwError(10, curLine, codePos, codePosAtLastNewLine);
                        }
                    }
                    else
                    {
                        throwError(10, curLine, codePos, codePosAtLastNewLine);
                    }
                }
                break;
            
            // equal sign (Complete)
            case 61:
                {
                    if(wordTypeExpected == 6)
                    {
                        wordTypeExpected = 7; // Expect an operand but not a label declaration
                        charCache_CharsStored = 0;
                    }
                    else
                    {
                        throwError(11, curLine, codePos, codePosAtLastNewLine);
                    }
                }
                break;

            // open parenthesis (start label reference) (Complete)
            case 40:
                {
                    if(wordTypeExpected == 0 || wordTypeExpected == 1 || wordTypeExpected == 7 || wordTypeExpected == 8)
                    {
                        if(charCache_CharsStored == 0)
                        {
                            if(wordTypeExpected == 0) // Reset 'outputTempStore' if a label reference is used without an instruction preceding it
                            {
                                outputTempStore = 0;
                            }

                            labelRefCache[labelRefCache_Stored][0] = outputCode_InstructionsStored; // Store the output position of the label reference
                            labelRefCache[labelRefCache_Stored][1] = codePos + 1; // Store the code starting position of the label reference about to be read
                            wordTypeExpected = 3;
                        }
                        else
                        {
                            throwError(13, curLine, codePos, codePosAtLastNewLine);
                        }
                    }
                    else
                    {
                        throwError(13, curLine, codePos, codePosAtLastNewLine);
                    }
                }
                break;

            // closed parenthesis (end label reference) (Complete)
            case 41:
                {
                    if(wordTypeExpected == 3 && charCache_CharsStored != 0)
                    {
                        labelRefCache[labelRefCache_Stored][2] = codePos - labelRefCache[labelRefCache_Stored][1];
                        labelRefCache_Stored += 1;
                        charCache_CharsStored = 0;
                        wordTypeExpected = 0;

                        // No default value is set for label references; therefore, data in "outputTempStore" must be saved to the output location right now
                        outputCode[outputCode_InstructionsStored] = outputTempStore;
                        outputCode_InstructionsStored += 1;
                    }
                    else
                    {
                        throwError(14, curLine, codePos, codePosAtLastNewLine);
                    }
                }
                break;
            
            // new line (Complete)
            case 10:
                {
                    curLine += 1;
                    codePosAtLastNewLine = codePos;
                }
                // no break statement, fallthrough to next case
            
            // semicolon (start comment) (Complete)
            case 59:
                {
                    if((int)charCache[charCache_CharsStored] == 59) // This must be asserted because this case is also executed when a new line character in encountered. This is unnecessary when programming in assembly because instead of new line cases falling through to this case, they would fall through to the default case.
                    {
                        if((wordTypeExpected == 0 && charCache_CharsStored == 0) || (wordTypeExpected == 4 || wordTypeExpected == 5))
                        {
                            // search for a new line character to end the comment
                            int i;
                            for(i = codePos; i < assemblyCodeEndPoint; i++)
                            {
                                if((int)assemblyCode[i] == 10)
                                {
                                    break;
                                }
                            }

                            codePos = i;
                            curLine += 1;
                            codePosAtLastNewLine = codePos;
                        }
                        else
                        {
                            throwError(1, curLine, codePos, codePosAtLastNewLine);
                        }
                    }
                }
                // no break statement, fallthrough to next case

            default: // (Complete)
                {
                    if(charCache_CharsStored != 0)
                    {
                        int magnitude = 1,
                            tempVar = 0;

                        // Operand is assumed to be complete and will be read
                        if(wordTypeExpected == 4) // Binary (Complete)
                        {
                            for(int i = charCache_CharsStored - 1; i >= 0; i--)
                            {
                                if((int)charCache[i] - 48 == 1)
                                {
                                    tempVar += magnitude;
                                }

                                magnitude *= 2;
                            }

                            outputTempStore += tempVar;
                        }
                        else if(wordTypeExpected == 5) // Decimal (Complete)
                        {
                            for(int i = charCache_CharsStored - 1; i >= 0; i--)
                            {
                                tempVar += ((int)charCache[i] - 48) * magnitude;

                                magnitude *= 10;
                            }

                            outputTempStore += tempVar;
                        }
                        else
                        {
                            throwError(9, curLine, codePos, codePosAtLastNewLine);
                        }

                        if(outputTempStore >= 65536)
                        {
                            throwError(15, curLine, codePos, codePosAtLastNewLine);
                        }

                        outputCode[outputCode_InstructionsStored] = outputTempStore;
                        outputCode_InstructionsStored += 1;
                        outputTempStore = 0;

                        charCache_CharsStored = 0;
                        wordTypeExpected = 0; // Expect another instruction after the previous one
                    }
                }
                break;
        }
    }

    /*
    // Temporary
    for(int i = 0; i < outputCode_InstructionsStored; i++)
    {
        std::cout << outputCode[i] << std::endl;
    }

    for(int i = 0; i < labelNameCache_NamesStored; i++)
    {
        for(int x = 0; x < labelNameCache_NameLength[i]; x++)
        {
            std::cout << labelNameCache[i][x];
        }
        std::cout << std::endl;
    }*/

    // Identify label references (NOT complete)
    int posDifference;
    bool matchFound = true;
    for(int i = 0; i < labelDefCache_Stored; i++)
    {
        for(int o = 0; o < labelRefCache_Stored; o++)
        {
            if(labelRefCache[o][2] == labelDefCache[i][2]) // Check if similarly sized names are the same
            {
                posDifference = labelDefCache[i][1] - labelRefCache[o][1];

                for(int k = labelRefCache[o][1]; k < labelRefCache[o][1] + labelRefCache[o][2]; k++)
                {
                    if(assemblyCode[k] != assemblyCode[k + posDifference])
                    {
                        matchFound = false;
                        break;
                    }
                }

                if(matchFound == true)
                {
                    if(labelRefCache_matched[o] == 1) // Check if this reference was already matched (if it was, this label declaration must be a duplicate)
                    {
                        throwError(16, 0, 0, 0);
                    }
                    else
                    {
                        outputCode[labelRefCache[o][0]] += labelDefCache[i][0]; // Store the location of the label declaration at the location of the label reference (which functions as a pointer)
                        labelRefCache_matched[o] = 1;
                    }
                }
                else
                {
                    matchFound = true;
                }
            }
        }
    }

    // Check if there are any label references with no declaration
    for(int i = 0; i < labelRefCache_Stored; i++)
    {
        if(labelRefCache_matched[i] == 0)
        {
            throwError(17, 0, 0, 0);
        }
    }

    if(charCache_CharsStored != 0)
    {
        throwError(6, curLine, assemblyCodeEndPoint - 1, codePosAtLastNewLine);
    }

    // Print output code to the console
    for(int i = 0; i < outputCode_InstructionsStored; i++)
    {
        std::cout << outputCode[i] << std::endl;
    }
}