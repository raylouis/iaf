/*===========================================================================*
 *                                                                           *
 *  sflmysql.h - MySQL database interface                                    *
 *                                                                           *
 *  Copyright (c) 1991-2002 iMatix Corporation                               *
 *                                                                           *
 *  ------------------ GPL Licensed Source Code ------------------           *
 *  iMatix makes this software available under the GNU General               *
 *  Public License (GPL) license for open source projects.  For              *
 *  details of the GPL license please see www.gnu.org or read the            *
 *  file license.gpl provided in this package.                               *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or            *
 *  modify it under the terms of the GNU General Public License as           *
 *  published by the Free Software Foundation; either version 2 of           *
 *  the License, or (at your option) any later version.                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public                *
 *  License along with this program in the file 'license.gpl'; if            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple              *
 *  Place - Suite 330, Boston, MA 02111-1307, USA.                           *
 *                                                                           *
 *  You can also license this software under iMatix's General Terms          *
 *  of Business (GTB) for commercial projects.  If you have not              *
 *  explicitly licensed this software under the iMatix GTB you may           *
 *  only use it under the terms of the GNU General Public License.           *
 *                                                                           *
 *  For more information, send an email to info@imatix.com.                  *
 *  --------------------------------------------------------------           *
 *===========================================================================*/
/*  ----------------------------------------------------------------<Prolog>-
    Synopsis:   Defines structures and constants for the mysql interface.
 ------------------------------------------------------------------</Prolog>-*/

#if defined (DBIO_MYSQL)

#ifndef _SFLMYSQL_INCLUDED
#define _SFLMYSQL_INCLUDED

#include "mysql.h"                      /*  Main MySQL header file           */
#include "mysqld_error.h"               /*  Error numbers                    */


typedef struct {
    COMMON_DBIO_CTX
    MYSQL     *db_handle;
    MYSQL_RES *result ;
    char  *table_name;                  /*  Table name                       */
} MYSQLHANDLE;

#ifdef __cplusplus
extern "C" {
#endif

void      *dbio_mysql_connect    (char *db_name, char *user, char *pwd,
                                  char *host);
void       dbio_mysql_disconnect (void *context);
int        dbio_mysql_commit     (void *context);
int        dbio_mysql_rollback   (void *context);
MYSQLHANDLE *alloc_mysql_handle  (char *table_name, void *context);
void       free_mysql_handle     (MYSQLHANDLE *handle);
void       set_mysql_connection  (void *context);

#ifdef __cplusplus
}
#endif

#endif
#endif
