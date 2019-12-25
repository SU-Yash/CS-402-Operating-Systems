/******************************************************************************/
/* Important CSCI 402 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 402 programming assignments at USC.              */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *
 * @(#)$Id: my402list.h,v 1.1 2019/05/19 23:34:12 cvsps Exp $
 */

#ifndef _MY402LIST_H_
#define _MY402LIST_H_

#include "cs402.h"

typedef struct tagMy402ListElem {
    void *obj;
    struct tagMy402ListElem *next;
    struct tagMy402ListElem *prev;
} My402ListElem;

typedef struct tagMy402List {
    int num_members;
    My402ListElem anchor;

    /* You do not have to set these function pointers */
    int  (*Length)(struct tagMy402List *);
    int  (*Empty)(struct tagMy402List *);

    int  (*Append)(struct tagMy402List *, void*);
    int  (*Prepend)(struct tagMy402List *, void*);
    void (*Unlink)(struct tagMy402List *, My402ListElem*);
    void (*UnlinkAll)(struct tagMy402List *);
    int  (*InsertBefore)(struct tagMy402List *, void*, My402ListElem*);
    int  (*InsertAfter)(struct tagMy402List *, void*, My402ListElem*);

    My402ListElem *(*First)(struct tagMy402List *);
    My402ListElem *(*Last)(struct tagMy402List *);
    My402ListElem *(*Next)(struct tagMy402List *, My402ListElem *cur);
    My402ListElem *(*Prev)(struct tagMy402List *, My402ListElem *cur);

    My402ListElem *(*Find)(struct tagMy402List *, void *obj);
} My402List;

extern int  My402ListLength(My402List*);
extern int  My402ListEmpty(My402List*);

extern int  My402ListAppend(My402List*, void*);
extern int  My402ListPrepend(My402List*, void*);
extern void My402ListUnlink(My402List*, My402ListElem*);
extern void My402ListUnlinkAll(My402List*);
extern int  My402ListInsertAfter(My402List*, void*, My402ListElem*);
extern int  My402ListInsertBefore(My402List*, void*, My402ListElem*);

extern My402ListElem *My402ListFirst(My402List*);
extern My402ListElem *My402ListLast(My402List*);
extern My402ListElem *My402ListNext(My402List*, My402ListElem*);
extern My402ListElem *My402ListPrev(My402List*, My402ListElem*);

extern My402ListElem *My402ListFind(My402List*, void*);

extern int My402ListInit(My402List*);

#endif /*_MY402LIST_H_*/
