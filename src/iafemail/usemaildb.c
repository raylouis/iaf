/*  ----------------------------------------------------------------<Prolog>-
    Name:       usemaildb.c
    Title:      UltraSource Email Daemon Database Access

    Generated by eslservice at 2003/02/19 11:49:18

    Synopsis:   Get data from email log and email queue

    This program is copyright (c) 1991-2001 iMatix Corporation.
    ---------------------------------------------------------------</prolog>-*/

#include "sfl.h"
#include <sqlext.h>
#include "usemaildb.h"

/*- Definition --------------------------------------------------------------*/

#define ERR_MSG_SIZE           300
#define ERR_CODE_SIZE          20

/*- Global Variable ---------------------------------------------------------*/

HENV  
    environment = NULL;                /*  ODBC environment                 */
HDBC
    connection  = NULL;                /*  ODBC connection handle           */
HSTMT
    log_stmt    = NULL,                /* ODBC Statement handle             */
    queue_stmt  = NULL;                /* ODBC Statement handle             */
RETCODE
    rc,
    retcode;
int
    feedback,                          /* feedback value                    */
    err_code;                          /* Error code ( 0 = NO ERROR)        */
static char
    err_code_msg [ERR_CODE_SIZE],      /* Error Message code                */
    err_message  [ERR_MSG_SIZE];       /* Error message                     */

    long   h_usid;
    long   h_uscustomerid;
    long   h_ususergroupid;
    long   h_usorderid;
    char   h_uscontext [50 + 1];
    long   h_ussender;
    char   h_usrecipients [255 + 1];
    char   h_ussubject [150 + 1];
    char   h_usfromaddr [255 + 1];
    char   h_usreplyaddr [255 + 1];
    long   h_usstatus;
    double h_ussentat;
    double h_uscreatedts;
    char   h_usbody [8000 + 1];
    char   h_usmessage [8000 + 1];
    long   h_usid;
    double h_ussendat;
    long   h_usemaillogid;
    double h_usrevisedts;

static SDWORD h_uscontext_ind
    ,h_usrecipients_ind
    ,h_ussubject_ind
    ,h_usfromaddr_ind
    ,h_usreplyaddr_ind
    ,h_usbody_ind
    ,h_usmessage_ind
    ;

/*- Local function declaration ---------------------------------------------*/

static int check_sql_error  (HSTMT statement);
void       prepare_log_data (USEMAILLOG *record);

/*##########################################################################*/
/*######################### COMMON FUNCTIONS ###############################*/
/*##########################################################################*/

/*  ---------------------------------------------------------------------[<]-
    Function: usemail_connect

    Synopsis: Connect to a odbc database.
    ---------------------------------------------------------------------[>]-*/

Bool 
usemail_connect (char *name, char *user, char *pwd)
{
    Bool
        feedback = FALSE;
    ASSERT (name);

    retcode = SQLAllocEnv (&environment); /* Environment handle              */
    if (retcode != SQL_SUCCESS)
      {
        coprintf ("Error: usemail_connect (SQLAllocEnv) of %s (%ld)",
                  name, retcode);
        return (FALSE);
      }
    retcode = SQLAllocConnect (environment, &connection);
    if (retcode == SQL_SUCCESS
    ||  retcode == SQL_SUCCESS_WITH_INFO)
      {
        /* Set login timeout to 30 seconds.                                  */
        SQLSetConnectOption (connection, SQL_LOGIN_TIMEOUT, 30);
        /* Connect to data source                                            */
        retcode = SQLConnect(connection, name, SQL_NTS, user, SQL_NTS,
                             pwd, SQL_NTS);
        if (retcode != SQL_SUCCESS
        &&  retcode != SQL_SUCCESS_WITH_INFO)
          {
            check_sql_error (NULL);
            SQLFreeConnect (connection);
            SQLFreeEnv     (environment);
          }
        else
            feedback = TRUE;
      }
    else
      {
        check_sql_error (NULL);
        SQLFreeEnv (environment);
      }
    return (feedback);
}

/*  ---------------------------------------------------------------------[<]-
    Function: usemail_disconnect

    Synopsis: 
    ---------------------------------------------------------------------[>]-*/

void
usemail_disconnect (void)
{
    SQLDisconnect  (connection);
    SQLFreeConnect (connection);
    SQLFreeEnv     (environment);
}

/*  ---------------------------------------------------------------------[<]-
    Function: usemail_error_message

    Synopsis: 
    ---------------------------------------------------------------------[>]-*/

char *
usemail_error_message (void)
{
    return (err_message);
}

/*##########################################################################*/
/*######################### USEMAILLOG FUNCTIONS #############################*/
/*##########################################################################*/

/*  ---------------------------------------------------------------------[<]-
    Function: usemaillog_update

    Synopsis: update a record into the table USEMAILLOG
    ---------------------------------------------------------------------[>]-*/
int
usemaillog_update (USEMAILLOG *record)
{
    static char update_buffer [] =
        "UPDATE USEMAILLOG SET "                                              \
            "  USCUSTOMERID       = ?"                                        \
            ", USUSERGROUPID      = ?"                                        \
            ", USORDERID          = ?"                                        \
            ", USCONTEXT          = ?"                                        \
            ", USSENDER           = ?"                                        \
            ", USRECIPIENTS       = ?"                                        \
            ", USSUBJECT          = ?"                                        \
            ", USFROMADDR         = ?"                                        \
            ", USREPLYADDR        = ?"                                        \
            ", USSTATUS           = ?"                                        \
            ", USSENTAT           = ?"                                        \
            ", USCREATEDTS        = ?"                                        \
        " WHERE ("                                                            \
            "USID                 = ?)";

    retcode = SQLAllocStmt (connection, &log_stmt);
    if (retcode != SQL_SUCCESS
    &&  retcode != SQL_SUCCESS_WITH_INFO)
      {
        check_sql_error (log_stmt);
        return (feedback);         
      }
    prepare_log_data (record);

    rc = SQLPrepare (log_stmt, update_buffer, SQL_NTS);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
      {
        rc  = SQLBindParameter (log_stmt, 1,                             SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_uscustomerid, 0,NULL);
        rc  = SQLBindParameter (log_stmt, 2,                             SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_ususergroupid, 0,NULL);
        rc  = SQLBindParameter (log_stmt, 3,                             SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_usorderid, 0,NULL);
        rc  = SQLBindParameter (log_stmt, 4,                             SQL_PARAM_INPUT,
                                SQL_C_CHAR,  SQL_LONGVARCHAR, 50, 0,
                                h_uscontext,    0, NULL);
        rc  = SQLBindParameter (log_stmt, 5,                             SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_ussender, 0,NULL);
        rc  = SQLBindParameter (log_stmt, 6,                             SQL_PARAM_INPUT,
                                SQL_C_CHAR,  SQL_LONGVARCHAR, 255, 0,
                                h_usrecipients,  0, NULL);
        rc  = SQLBindParameter (log_stmt, 7,                             SQL_PARAM_INPUT,
                                SQL_C_CHAR,  SQL_LONGVARCHAR, 150, 0,
                                h_ussubject,    0, NULL);
        rc  = SQLBindParameter (log_stmt, 8,                             SQL_PARAM_INPUT,
                                SQL_C_CHAR,  SQL_LONGVARCHAR, 255, 0,
                                h_usfromaddr,   0, NULL);
        rc  = SQLBindParameter (log_stmt, 9,                             SQL_PARAM_INPUT,
                                SQL_C_CHAR,  SQL_LONGVARCHAR, 255, 0,
                                h_usreplyaddr,  0, NULL);
        rc  = SQLBindParameter (log_stmt, 10,                            SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_usstatus, 0,NULL);
        rc  = SQLBindParameter (log_stmt, 11,                            SQL_PARAM_INPUT,
                                SQL_C_DOUBLE,  SQL_DOUBLE, 0, 0,
                                &h_ussentat,     0, NULL);
        rc  = SQLBindParameter (log_stmt, 12,                            SQL_PARAM_INPUT,
                                SQL_C_DOUBLE,  SQL_DOUBLE, 0, 0,
                                &h_uscreatedts,  0, NULL);
        rc  = SQLBindParameter (log_stmt, 13,                            SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_usid, 0,NULL);

        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            rc = SQLExecute (log_stmt);
      }
    check_sql_error (log_stmt);

    SQLFreeStmt (log_stmt, SQL_DROP);
    return (feedback);
}

/*  ---------------------------------------------------------------------[<]-
    Function: usemaillog_get

    Synopsis: get a record from the table USEMAILLOG
    ---------------------------------------------------------------------[>]-*/

int
usemaillog_get (USEMAILLOG *record)
{
    static char select_statement [] =
        "SELECT "                                                             \
         "  USID"                                                             \
         ", USCUSTOMERID"                                                     \
         ", USUSERGROUPID"                                                    \
         ", USORDERID"                                                        \
         ", USCONTEXT"                                                        \
         ", USSENDER"                                                         \
         ", USRECIPIENTS"                                                     \
         ", USSUBJECT"                                                        \
         ", USFROMADDR"                                                       \
         ", USREPLYADDR"                                                      \
         ", USSTATUS"                                                         \
         ", USSENTAT"                                                         \
         ", USCREATEDTS"                                                      \
         ", USBODY"                                                           \
         ", USMESSAGE"                                                        \
         " FROM USEMAILLOG WHERE"                                             \
         "(USID = ? )";
    SDWORD
        indicator;

    prepare_log_data (record);

    retcode = SQLAllocStmt (connection, &log_stmt);
    if (retcode != SQL_SUCCESS
    &&  retcode != SQL_SUCCESS_WITH_INFO)
      {
        check_sql_error (log_stmt);
        return (feedback);         
      }
    rc = SQLPrepare (log_stmt, select_statement, SQL_NTS);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
      {
        rc  = SQLBindParameter (log_stmt, 1,                             SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_usid, 0,NULL);
        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            rc = SQLExecute (log_stmt);
      }
    check_sql_error (log_stmt);
    if (feedback == OK)
      {
        rc = SQLBindCol (log_stmt, 1, 
                         SQL_C_LONG,   &h_usid, 0, &indicator);
        rc = SQLBindCol (log_stmt, 2, 
                         SQL_C_LONG,   &h_uscustomerid, 0, &indicator);
        rc = SQLBindCol (log_stmt, 3, 
                         SQL_C_LONG,   &h_ususergroupid, 0, &indicator);
        rc = SQLBindCol (log_stmt, 4, 
                         SQL_C_LONG,   &h_usorderid, 0, &indicator);
        rc = SQLBindCol (log_stmt, 5, 
                         SQL_C_CHAR,   &h_uscontext, 50 + 1, &h_uscontext_ind);
        rc = SQLBindCol (log_stmt, 6, 
                         SQL_C_LONG,   &h_ussender, 0, &indicator);
        rc = SQLBindCol (log_stmt, 7, 
                         SQL_C_CHAR,   &h_usrecipients, 255 + 1, &h_usrecipients_ind);
        rc = SQLBindCol (log_stmt, 8, 
                         SQL_C_CHAR,   &h_ussubject, 150 + 1, &h_ussubject_ind);
        rc = SQLBindCol (log_stmt, 9, 
                         SQL_C_CHAR,   &h_usfromaddr, 255 + 1, &h_usfromaddr_ind);
        rc = SQLBindCol (log_stmt, 10, 
                         SQL_C_CHAR,   &h_usreplyaddr, 255 + 1, &h_usreplyaddr_ind);
        rc = SQLBindCol (log_stmt, 11, 
                         SQL_C_LONG,   &h_usstatus, 0, &indicator);
        rc = SQLBindCol (log_stmt, 12, 
                         SQL_C_DOUBLE, &h_ussentat, 0, &indicator);
        rc = SQLBindCol (log_stmt, 13, 
                         SQL_C_DOUBLE, &h_uscreatedts, 0, &indicator);
        rc = SQLBindCol (log_stmt, 14, 
                         SQL_C_CHAR,   &h_usbody, 8000 + 1, &h_usbody_ind);
        rc = SQLBindCol (log_stmt, 15, 
                         SQL_C_CHAR,   &h_usmessage, 8000 + 1, &h_usmessage_ind);
        rc = SQLFetch (log_stmt);
        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
          {
            check_sql_error (log_stmt);
            feedback = RECORD_NOT_PRESENT;
          }
        else
          {
            memset (record, 0, sizeof (USEMAILLOG));
            record-> usid                 = h_usid;
            record-> uscustomerid         = h_uscustomerid;
            record-> ususergroupid        = h_ususergroupid;
            record-> usorderid            = h_usorderid;
            record-> usstatus             = h_usstatus;
            record-> ussender             = h_ussender;
            if (h_usrecipients_ind < 0)
                *h_usrecipients                      = '\0';
            else
                h_usrecipients [h_usrecipients_ind]  = '\0';
            strcrop (h_usrecipients);
            strcpy (record-> usrecipients,        h_usrecipients);
            if (h_ussubject_ind < 0)
                *h_ussubject                         = '\0';
            else
                h_ussubject [h_ussubject_ind]        = '\0';
            strcrop (h_ussubject);
            strcpy (record-> ussubject,           h_ussubject);
            if (h_usfromaddr_ind < 0)
                *h_usfromaddr                        = '\0';
            else
                h_usfromaddr [h_usfromaddr_ind]      = '\0';
            strcrop (h_usfromaddr);
            strcpy (record-> usfromaddr,          h_usfromaddr);
            if (h_usreplyaddr_ind < 0)
                *h_usreplyaddr                       = '\0';
            else
                h_usreplyaddr [h_usreplyaddr_ind]    = '\0';
            strcrop (h_usreplyaddr);
            strcpy (record-> usreplyaddr,         h_usreplyaddr);
            if (h_uscontext_ind < 0)
                *h_uscontext                         = '\0';
            else
                h_uscontext [h_uscontext_ind]        = '\0';
            strcrop (h_uscontext);
            strcpy (record-> uscontext,           h_uscontext);
            if (h_usmessage_ind < 0)
                *h_usmessage                         = '\0';
            else
                h_usmessage [h_usmessage_ind]        = '\0';
            strcrop (h_usmessage);
            strcpy (record-> usmessage,           h_usmessage);
            if (h_usbody_ind < 0)
                *h_usbody                            = '\0';
            else
                h_usbody [h_usbody_ind]              = '\0';
            strcrop (h_usbody);
            strcpy (record-> usbody,              h_usbody);
            record-> uscreatedts          = h_uscreatedts;
            record-> ussentat             = h_ussentat;
          }
      }
    SQLFreeStmt (log_stmt, SQL_DROP);
    return (feedback);
}

/*##########################################################################*/
/*######################## EMAILQUEUE FUNCTIONS ############################*/
/*##########################################################################*/

/*  ---------------------------------------------------------------------[<]-
    Function: usemailqueue_get

    Synopsis: Get all email queue record in alocated record table
    Return the number of record in table.
    ---------------------------------------------------------------------[>]-*/

long
usemailqueue_get_all (double timestamp, USEMAILQUEUE **record)
{
    static char select_statement [] =
        "SELECT USID, USSENDAT, USEMAILLOGID, USREVISEDTS FROM USEMAILQUEUE " \
        "WHERE (USSENDAT <= ?) ORDER BY USSENDAT ASC";
    static char count_statement [] =                                          \
         "SELECT count (*) FROM USEMAILQUEUE WHERE (USSENDAT <= ?)";
    SDWORD
        indicator;
    long
        rec_count = 0;
    USEMAILQUEUE
        *rec;

    h_ussendat = timestamp; 
    retcode = SQLAllocStmt (connection, &queue_stmt);
    if (retcode != SQL_SUCCESS
    &&  retcode != SQL_SUCCESS_WITH_INFO)
      {
        check_sql_error (queue_stmt);
        return (feedback);         
      }

    /* Get count of record                                                   */
    rc = SQLPrepare (queue_stmt, count_statement, SQL_NTS);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
      {
        rc  = SQLBindParameter (queue_stmt, 1,                           SQL_PARAM_INPUT,
                                SQL_C_DOUBLE,  SQL_DOUBLE, 0, 0,
                                &h_ussendat,     0, NULL);
        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            rc = SQLExecute (queue_stmt);
        check_sql_error (queue_stmt);
      }
    if (feedback == OK)
      {
        rc = SQLBindCol (queue_stmt, 1, 
                         SQL_C_LONG,   &rec_count, 0, &indicator);
        rc = SQLFetch (queue_stmt);
        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
          {
            check_sql_error (queue_stmt);
            feedback = RECORD_NOT_PRESENT;
          }
      }


    if (rec_count > 0)
      {
        *record = mem_alloc (sizeof (USEMAILQUEUE) * rec_count);
        if (*record)
          {
            memset (*record, 0, sizeof (USEMAILQUEUE) * rec_count);

            SQLFreeStmt (queue_stmt, SQL_CLOSE);
            rc = SQLPrepare (queue_stmt, select_statement, SQL_NTS);
            if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
             {
        rc  = SQLBindParameter (queue_stmt, 1,                           SQL_PARAM_INPUT,
                                SQL_C_DOUBLE,  SQL_DOUBLE, 0, 0,
                                &h_ussendat,     0, NULL);
               if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
                  rc = SQLExecute (queue_stmt);
               check_sql_error (queue_stmt);
             }
            rec = *record;
            while (feedback == OK)
             {
        rc = SQLBindCol (queue_stmt, 1, 
                         SQL_C_LONG,   &h_usid, 0, &indicator);
        rc = SQLBindCol (queue_stmt, 2, 
                         SQL_C_DOUBLE, &h_ussendat, 0, &indicator);
        rc = SQLBindCol (queue_stmt, 3, 
                         SQL_C_LONG,   &h_usemaillogid, 0, &indicator);
        rc = SQLBindCol (queue_stmt, 4, 
                         SQL_C_DOUBLE, &h_usrevisedts, 0, &indicator);
                rc = SQLFetch (queue_stmt);
                if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
                  {
                    check_sql_error (queue_stmt);
                    feedback = RECORD_NOT_PRESENT;
                  }
                else
                  {
            rec-> usid                    = h_usid;
            rec-> usemaillogid            = h_usemaillogid;
            rec-> ussendat                = h_ussendat;
            rec-> usrevisedts             = h_usrevisedts;
                  }
                rec++;
             }
          }
      }
    SQLFreeStmt (queue_stmt, SQL_DROP);

    return (rec_count);
}


/*  ---------------------------------------------------------------------[<]-
    Function: usemailqueue_delete

    Synopsis: delete a record into the table USEMAILQUEUE
    ---------------------------------------------------------------------[>]-*/

int
usemailqueue_delete      (USEMAILQUEUE *record)
{
    HSTMT
        queue_stmt;
    static char delete_record [] =
        "DELETE FROM USEMAILQUEUE WHERE ( "                                  \
            "USID = ?)";
    h_usid                = record-> usid; 


    retcode = SQLAllocStmt (connection, &queue_stmt);
    if (retcode != SQL_SUCCESS
     &&  retcode != SQL_SUCCESS_WITH_INFO)
      {
        check_sql_error (queue_stmt);
        return (feedback);         
      }

    rc = SQLPrepare (queue_stmt, delete_record, SQL_NTS);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
      {
        rc  = SQLBindParameter (queue_stmt, 1,                           SQL_PARAM_INPUT,
                                SQL_C_LONG,  SQL_INTEGER, 0, 0,
                                &h_usid, 0,NULL);
        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
            rc = SQLExecute (queue_stmt);
      }
    check_sql_error (queue_stmt);
    SQLFreeStmt (queue_stmt, SQL_DROP);
    return (feedback);         
}

/*##########################################################################*/
/*######################### INTERNAL FUNCTIONS #############################*/
/*##########################################################################*/

/*  ------------------------------------------------------------------------
    Function: check_sql_error - INTERNAL

    Synopsis: Check the SQL error
    ------------------------------------------------------------------------*/

static int
check_sql_error (HSTMT statement)
{
    long
        size;
    short
        size2;

    err_code = 0;
    
    if (rc == SQL_SUCCESS
    ||  rc == SQL_SUCCESS_WITH_INFO)
        err_code = 0;
    else
      {
        SQLError (environment, connection, statement, err_code_msg, &size,
                  err_message, ERR_MSG_SIZE - 1, &size2);
        if (strcmp (err_code_msg, "23000") == 0)
            err_code = -1;
        else
        if (strcmp (err_code_msg, "00000") == 0)
            err_code = 1;
        else
            err_code = -2;
      }

    if (err_code > 0)
      {
        feedback = RECORD_NOT_PRESENT;
      }
    else
    if (err_code == -1)
      {
        feedback = DUPLICATE_RECORD;
      }
    else
    if (err_code < 0)
      {
        feedback = HARD_ERROR;
        coprintf ("SQL Error    %d : %s", err_code, err_message);
      }
    else
        feedback = OK;

    return (feedback);
}


/*  ------------------------------------------------------------------------
    Function: prepare_log_data - INTERNAL

    Synopsis:
    ------------------------------------------------------------------------*/

static void
prepare_log_data (USEMAILLOG *record)
{
    h_usid                 = record-> usid; 
    h_uscustomerid         = record-> uscustomerid; 
    h_ususergroupid        = record-> ususergroupid; 
    h_usorderid            = record-> usorderid; 
    h_usstatus             = record-> usstatus; 
    h_ussender             = record-> ussender; 
    strcpy (h_usrecipients,  record-> usrecipients);
    strcpy (h_ussubject,     record-> ussubject);
    strcpy (h_usfromaddr,    record-> usfromaddr);
    strcpy (h_usreplyaddr,   record-> usreplyaddr);
    strcpy (h_uscontext,     record-> uscontext);
    strcpy (h_usmessage,     record-> usmessage);
    strcpy (h_usbody,        record-> usbody);
    h_uscreatedts          = record-> uscreatedts; 
    h_ussentat             = record-> ussentat; 
}