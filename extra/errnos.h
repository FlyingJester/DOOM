#ifndef ERRNOS_HEAD
#define ERRNOS_HEAD
/* PROGRAM:     all
 * FILE:        $Header: /home/ghn/projects/PEAR/egg/eggsh/RCS/errnos.h,v 1.1 1998/07/21 11:39:53 ghn Exp $
 * PURPOSE:     Definitions for error constants
 * AUTHOR:      Greg Nelson
 * DATE:        98-05-09
 *
 * REVISED:     $Log: errnos.h,v $
 * REVISED:     Revision 1.1  1998/07/21 11:39:53  ghn
 * REVISED:     Initial revision
 * REVISED:
 * Copyright 1998 - Greg Nelson
 * Redistributable under the terms of the GNU Public Licence (GPL)
 */

/* 0 - no error */
#define ERR_NONE        0

/* -1 to -99 - generic errors */
#define ERR_OTHER       -1      /* Catchall for other errors */
#define ERR_NOMEM       -2      /* Could not allocate memory            */
#define ERR_NIMP        -3      /* Not implemented */
#define ERR_WRONGARGN   -4      /* Wrong number of arguments.           */
#define ERR_INRANGE     -5      /* Input out of range */
#define ERR_OTIMEOUT    -6      /* Got a timeout, not from COMM         */
#define ERR_STRTOOLONG  -7      /* String was longer than allowed       */

/* -100 to -199 - comm errors */
#define ERR_STRAYINPUT  -100    /* Stray characters were received       */
#define ERR_PKT_BADSTX  -101    /* Packet did not begin with STX        */
#define ERR_PKT_BADLEN  -102    /* Length word could not be read        */
#define ERR_PKT_SHORT   -103    /* Packet was not long enough           */
#define ERR_PKT_CKSUM   -104    /* Packet checksum did not match        */
#define ERR_PKT_BADETX  -105    /* Packet did not end with ETX          */
#define ERR_COMM_TMOUT  -106    /* Timeout during communication         */
#define ERR_COMM_BRK    -107    /* Received break character             */
#define ERR_COMM_PARITY -108    /* Received parity error                */
#define ERR_COMM_FRAME  -109    /* Received framing error               */
#define ERR_COMM_OVERRN -110    /* Received overrun error               */
#define ERR_BUFF_FULL   -111    /* Buffer was too full to queue msg     */
#define ERR_NOREPLY     -112    /* Device did not reply to hails        */
#define ERR_RESPINVAL   -113    /* Response was invalid                 */
#define ERR_COMM_BTMOUT -114    /* Timeout inside body of message       */
#define ERR_PORT_INACT  -115    /* Attempt to use an inactive port      */

/* -200 to -299 - math errors */
#define ERR_DIV0        -200    /* Attempt to divide by zero            */
#define ERR_NOSOLN      -201    /* Couldn't solve for discontinuities   */
#define ERR_APPROX      -202    /* Result was an inexact approximation  */
#define ERR_NOVARS      -203    /* No parameters were variable          */
#define ERR_SINGULAR    -204    /* Matrix was singular, unsolvable      */
#define ERR_OVERFLOW    -205    /* Numerical overflow                   */

/* -300 to -399 - DLL specific errors */
#define ERR_NOTSUPP     -300    /* Current device does not support fn   */
#define ERR_RESOURCE    -301    /* Ran out of a DLL limited resource    */
#define ERR_DEVDRVR     -302    /* Windows device driver returned err   */
#define ERR_DEVSELECT   -303    /* Selecting a device failed (general)  */
#define ERR_NOENT       -320    /* File not found                       */
#define ERR_CNREAD      -321    /* Couldn't read desired data from file */
#define ERR_CNWRITE     -322    /* Couldn't write desired data to file  */
#define ERR_EOF         -323    /* End of file reached unexpectedly     */
#define ERR_DLGCAN      -324    /* Dialog canceled, not an "error"      */

#endif
