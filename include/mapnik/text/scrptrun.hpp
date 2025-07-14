/*
 *******************************************************************************
 *
 *   Copyright (C) 1999-2003, International Business Machines
 *   Corporation and others.  All Rights Reserved.
 *
 *******************************************************************************
 *   file name:  scrptrun.h
 *
 *   created on: 10/17/2001
 *   created by: Eric R. Mader
 *
 * NOTE: This file is copied from ICU.
 * http://source.icu-project.org/repos/icu/icu/trunk/license.html
 */

#ifndef __SCRPTRUN_H
#define __SCRPTRUN_H

#include <mapnik/config.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <unicode/utypes.h>
#include <unicode/uobject.h>
#include <unicode/uscript.h>
MAPNIK_DISABLE_WARNING_POP
#include <vector>

struct ScriptRecord
{
    UChar32 startChar = 0;
    UChar32 endChar = 0;
    UScriptCode scriptCode = USCRIPT_INVALID_CODE;
};

struct ParenStackEntry
{
    ParenStackEntry(int32_t pairIndex_, UScriptCode scriptCode_)
        : pairIndex(pairIndex_),
          scriptCode(scriptCode_)
    {}
    int32_t pairIndex = 0;
    UScriptCode scriptCode = USCRIPT_INVALID_CODE;
};

class MAPNIK_DECL ScriptRun : public icu::UObject
{
  public:
    ScriptRun();

    ScriptRun(UChar const chars[], int32_t length);

    ScriptRun(UChar const chars[], int32_t start, int32_t length);

    void reset();

    void reset(int32_t start, int32_t count);

    void reset(UChar const chars[], int32_t start, int32_t length);

    int32_t getScriptStart();

    int32_t getScriptEnd();

    UScriptCode getScriptCode();

    UBool next();

    /**
     * ICU "poor man's RTTI", returns a UClassID for the actual class.
     *
     * @stable ICU 2.2
     */
    virtual inline UClassID getDynamicClassID() const { return getStaticClassID(); }

    /**
     * ICU "poor man's RTTI", returns a UClassID for this class.
     *
     * @stable ICU 2.2
     */
    static inline UClassID getStaticClassID() { return (UClassID)&fgClassID; }

  private:

    static UBool sameScript(int32_t scriptOne, int32_t scriptTwo);

    int32_t charStart;
    int32_t charLimit;
    UChar const* charArray;

    int32_t scriptStart;
    int32_t scriptEnd;
    UScriptCode scriptCode;

    std::vector<ParenStackEntry> parenStack;
    int32_t parenSP;

    static int8_t highBit(int32_t value);
    static int32_t getPairIndex(UChar32 ch);

    static UChar32 pairedChars[];
    static int32_t const pairedCharCount;
    static int32_t const pairedCharPower;
    static int32_t const pairedCharExtra;

    /**
     * The address of this static class variable serves as this class's ID
     * for ICU "poor man's RTTI".
     */
    static char const fgClassID;
    // initial stack size
    unsigned int const STACK_SIZE = 1 << 4; // 2^n
};

inline ScriptRun::ScriptRun()
{
    parenStack.reserve(STACK_SIZE);
    reset(nullptr, 0, 0);
}

inline ScriptRun::ScriptRun(UChar const chars[], int32_t length)
{
    parenStack.reserve(STACK_SIZE);
    reset(chars, 0, length);
}

inline ScriptRun::ScriptRun(UChar const chars[], int32_t start, int32_t length)
{
    parenStack.reserve(STACK_SIZE);
    reset(chars, start, length);
}

inline int32_t ScriptRun::getScriptStart()
{
    return scriptStart;
}

inline int32_t ScriptRun::getScriptEnd()
{
    return scriptEnd;
}

inline UScriptCode ScriptRun::getScriptCode()
{
    return scriptCode;
}

inline void ScriptRun::reset()
{
    scriptStart = charStart;
    scriptEnd = charStart;
    scriptCode = USCRIPT_INVALID_CODE;
    parenSP = -1;
}

inline void ScriptRun::reset(int32_t start, int32_t length)
{
    charStart = start;
    charLimit = start + length;

    reset();
}

inline void ScriptRun::reset(UChar const chars[], int32_t start, int32_t length)
{
    charArray = chars;

    reset(start, length);
}

#endif
