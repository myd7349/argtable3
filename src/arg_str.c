/*******************************************************************************
 * arg_str: Implements the str command-line option
 *
 * This file is part of the argtable3 library.
 *
 * Copyright (C) 1998-2001,2003-2011,2013 Stewart Heitmann
 * <sheitmann@users.sourceforge.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of STEWART HEITMANN nor the  names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL STEWART HEITMANN BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include "argtable3.h"

#ifndef ARG_AMALGAMATION
#include "argtable3_private.h"
#endif

#include <stdlib.h>

static void arg_str_resetfn(struct arg_str* parent) {
    ARG_TRACE(("%s:resetfn(%p)\n", __FILE__, parent));
    parent->count = 0;
}

static int arg_str_scanfn(struct arg_str* parent, const char* argval) {
    int errorcode = 0;

    if (parent->count == parent->hdr.maxcount) {
        /* maximum number of arguments exceeded */
        errorcode = ARG_ERR_MAXCOUNT;
    } else if (!argval) {
        /* a valid argument with no argument value was given. */
        /* This happens when an optional argument value was invoked. */
        /* leave parent argument value unaltered but still count the argument. */
        parent->count++;
    } else {
        parent->sval[parent->count++] = argval;
    }

    ARG_TRACE(("%s:scanfn(%p) returns %d\n", __FILE__, parent, errorcode));
    return errorcode;
}

static int arg_str_checkfn(struct arg_str* parent) {
    int errorcode = (parent->count < parent->hdr.mincount) ? ARG_ERR_MINCOUNT : 0;

    ARG_TRACE(("%s:checkfn(%p) returns %d\n", __FILE__, parent, errorcode));
    return errorcode;
}

static void arg_str_errorfn(struct arg_str* parent, arg_dstr_t ds, int errorcode, const char* argval, const char* progname) {
    const char* shortopts = parent->hdr.shortopts;
    const char* longopts = parent->hdr.longopts;
    const char* datatype = parent->hdr.datatype;

    /* make argval NULL safe */
    argval = argval ? argval : "";

    arg_dstr_catf(ds, "%s: ", progname);
    switch (errorcode) {
        case ARG_ERR_MINCOUNT:
            arg_dstr_cat(ds, "missing option ");
            arg_print_option_ds(ds, shortopts, longopts, datatype, "\n");
            break;

        case ARG_ERR_MAXCOUNT:
            arg_dstr_cat(ds, "excess option ");
            arg_print_option_ds(ds, shortopts, longopts, argval, "\n");
            break;
    }
}

struct arg_str* arg_str0(const char* shortopts, const char* longopts, const char* datatype, const char* glossary) {
    return arg_strn(shortopts, longopts, datatype, 0, 1, glossary);
}

struct arg_str* arg_str1(const char* shortopts, const char* longopts, const char* datatype, const char* glossary) {
    return arg_strn(shortopts, longopts, datatype, 1, 1, glossary);
}

struct arg_str* arg_strn(const char* shortopts, const char* longopts, const char* datatype, int mincount, int maxcount, const char* glossary) {
    size_t nbytes;
    struct arg_str* result;

    /* should not allow this stupid error */
    /* we should return an error code warning this logic error */
    /* foolproof things by ensuring maxcount is not less than mincount */
    maxcount = (maxcount < mincount) ? mincount : maxcount;

    nbytes = sizeof(struct arg_str)      /* storage for struct arg_str */
             + maxcount * sizeof(char*); /* storage for sval[maxcount] array */

    result = (struct arg_str*)xmalloc(nbytes);

    int i;

    /* init the arg_hdr struct */
    result->hdr.flag = ARG_HASVALUE;
    result->hdr.shortopts = shortopts;
    result->hdr.longopts = longopts;
    result->hdr.datatype = datatype ? datatype : "<string>";
    result->hdr.glossary = glossary;
    result->hdr.mincount = mincount;
    result->hdr.maxcount = maxcount;
    result->hdr.parent = result;
    result->hdr.resetfn = (arg_resetfn*)arg_str_resetfn;
    result->hdr.scanfn = (arg_scanfn*)arg_str_scanfn;
    result->hdr.checkfn = (arg_checkfn*)arg_str_checkfn;
    result->hdr.errorfn = (arg_errorfn*)arg_str_errorfn;

    /* store the sval[maxcount] array immediately after the arg_str struct */
    result->sval = (const char**)(result + 1);
    result->count = 0;

    /* foolproof the string pointers by initializing them to reference empty strings */
    for (i = 0; i < maxcount; i++)
        result->sval[i] = "";

    ARG_TRACE(("arg_strn() returns %p\n", result));
    return result;
}
