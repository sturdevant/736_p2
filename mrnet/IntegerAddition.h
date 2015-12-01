/****************************************************************************
 *
 * Copyright © 2003-2015 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(integer_addition_h )
#define integer_addition_h 1

#include "mrnet/Types.h"

typedef enum { PROT_EXIT=FirstApplicationTag, 
               PROT_INIT,
               //PROT_STREAM,
               //PROT_PTREQ,
               //PROT_POLYREQ,
               //PROT_SUM,
               //PROT_NAME,
               PROT_REQUEST,
               PROT_RESPONSE} Protocol;

#endif /* integer_addition_h */
