//
// Created by lonnyliu(刘卓) on 2019-04-11.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include "MysqlUtil.h"
#include "Logging.h"
#include "Mutex.h"
#include <mysql.h>

using namespace std;

namespace Comm{

    struct Mysql::MysqlProp
    {
        string sErrMsg;
        char szHost[64];
        char szUser[64];
        char szPass[64];
        char szDB[256];     //db name.
        int iPort;
        char szNULL[1];
        unsigned int  iField;
        unsigned int  iRows;
        MYSQL connection;
        MYSQL_RES *result;
        MYSQL_ROW row;
        STRING2INT FieldIndex;
        vector<string> FieldNameList;
        bool bFieldIndexInitialized;
        bool bConnected;
        int iRowNum;
        int iErrCode;
    };

    Mysql:: Mysql()
    {
        InitLibrary();
        m_pProp = new MysqlProp();
        mysql_init (&m_pProp->connection);
        m_pProp->bConnected = false;
        m_pProp->iField = 0;
        m_pProp->iRows = 0;
        m_pProp->result = NULL;
        bzero (m_pProp->szHost, sizeof(m_pProp->szHost));
        bzero (m_pProp->szUser, sizeof(m_pProp->szUser));
        bzero (m_pProp->szPass, sizeof(m_pProp->szPass));
        bzero (m_pProp->szDB, sizeof(m_pProp->szDB));
        bzero (m_pProp->szNULL, sizeof(m_pProp->szNULL));
        m_pProp->iPort = 0;
        m_pProp->bFieldIndexInitialized = false;
        m_pProp->iRowNum = -1;
        m_pProp->iErrCode = 0;
    }

    Mysql:: Mysql(const char* szHost, const char* szUser, const char* szPass, const char* szDB, int iPort)
    {
        InitLibrary();
        m_pProp = new MysqlProp();
        mysql_init(&m_pProp->connection);
        m_pProp->bConnected = false;
        m_pProp->iField = 0;
        m_pProp->iRows = 0;
        m_pProp->result = NULL;
        strncpy (m_pProp->szHost, szHost, sizeof(m_pProp->szHost));
        strncpy (m_pProp->szUser, szUser, sizeof(m_pProp->szUser));
        strncpy (m_pProp->szPass, szPass, sizeof(m_pProp->szPass));
        strncpy (m_pProp->szDB, szDB, sizeof(m_pProp->szDB));
        bzero (m_pProp->szNULL, sizeof(m_pProp->szNULL));
        m_pProp->iPort = iPort;
        m_pProp->bFieldIndexInitialized = false;
        m_pProp->iRowNum = -1;
        m_pProp->iErrCode = 0;
    }

    Mysql:: ~Mysql()
    {
        if (m_pProp->bConnected)
        {
            FreeResult();
            mysql_close(&m_pProp->connection);
        }
        delete m_pProp;
    }

    void Mysql::InitLibrary()
    {
        static bool init = false;
        MutexLock mutex;
        if (!init)
        {
            MutexLockGuard lock(mutex);
            if (!init)
            {
                mysql_library_init(0, NULL, NULL);
                init = true;
            }
        }
    }

    const char* Mysql::getErrMsg() const
    {
        return m_pProp->sErrMsg.c_str();
    }

    int Mysql::getErrCode(void) const
    {
        return m_pProp->iErrCode;
    }

    int Mysql::StripField(string& to, const char* from,unsigned long len)
    {
        if(from==NULL) return -1;

        char *tostr = new char[2*len+1];

        mysql_real_escape_string(&m_pProp->connection,tostr,from,len);
        to = tostr;
        delete [] tostr;
        return 0;
    }

    bool Mysql:: IsConnected()
    {
        return m_pProp->bConnected;
    }

    int Mysql:: SetCharset(const char * sCharset)
    {
        return mysql_set_character_set(&m_pProp->connection, sCharset);
    }

    const char * Mysql:: CharacterSetName()
    {
        return mysql_character_set_name(&m_pProp->connection);
    }

    int Mysql:: Connect(const char* szHost, const char* szUser, const char* szPass,
                        const char* szDB, int iPort)
    {
        strncpy (m_pProp->szHost, szHost, sizeof(m_pProp->szHost));
        strncpy (m_pProp->szUser, szUser, sizeof(m_pProp->szUser));
        strncpy (m_pProp->szPass, szPass, sizeof(m_pProp->szPass));
        strncpy(m_pProp->szDB,szDB,sizeof(m_pProp->szDB));
        m_pProp->iPort = iPort;
        return Connect();
    }



    int Mysql:: Connect()
    {
        if (!m_pProp->bConnected)
        {
            char *dbname;
            if(m_pProp->szDB[0]=='\0') dbname= NULL;
            else dbname = m_pProp->szDB;
            if (mysql_real_connect(&m_pProp->connection, m_pProp->szHost,
                    m_pProp->szUser, m_pProp->szPass, dbname, m_pProp->iPort, NULL, 0) == NULL)
            {
                m_pProp->sErrMsg = string("connect to mysql failed") + string(mysql_error(&m_pProp->connection));
                m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
                LOG_ERROR << m_pProp->sErrMsg;
                return -1;
            }
            m_pProp->bConnected = true;
        }
        return 0;
    }

    int Mysql:: SetOption(mysql_option option, const char *arg)
    {
        return mysql_options(&m_pProp->connection, option, arg);
    }

    int Mysql:: SetServerOption(enum_mysql_set_option option)
    {
        return mysql_set_server_option(&m_pProp->connection, option);
    }

    int Mysql:: MysqlQuery( MYSQL * ptMysql, const char * sSql )
    {
        int ret = mysql_query( ptMysql, sSql );

        if( 0 != ret )
        {

            if( 0 == mysql_ping( ptMysql ) )
            {
                ret = mysql_query( ptMysql, sSql );
            }
        }
        if (0 != ret)
        {
            LOG_ERROR <<  "ERR: mysql_query: " <<  mysql_error( ptMysql );
        }
        return ret;
    }

    int Mysql:: Query(const char* szSqlString)
    {
        Connect();
        if(mysql_ping(&m_pProp->connection) != 0)
        {
            FreeResult();
            mysql_close(&m_pProp->connection);
            m_pProp->bConnected = false;
            if(0 != Connect()) {
                m_pProp->bConnected = false;
                m_pProp->sErrMsg = string("mysql connection broken:"), string(mysql_error(&m_pProp->connection));
                m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
                LOG_ERROR << m_pProp->sErrMsg;
                return -1;
            }
        }
        //int size = strlen(szSqlString);
        if (MysqlQuery(&m_pProp->connection, szSqlString) != 0)
        {
            m_pProp->sErrMsg = string("mysql query failed:") + string(mysql_error(&m_pProp->connection));
            m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
            LOG_ERROR << m_pProp->sErrMsg;
            return -2;
        }
        if(StoreResult() != 0)
        {
            return -3;
        }
        return 0;
    }

    int Mysql:: QueryWithoutReconnect(const char* szSqlString)
    {
        //int size = strlen(szSqlString);
        if (MysqlQuery(&m_pProp->connection, szSqlString) != 0)
        {
            m_pProp->sErrMsg = string("mysql query failed:") + string(mysql_error(&m_pProp->connection));
            m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
            LOG_ERROR << m_pProp->sErrMsg;
            return -2;
        }
        if(StoreResult() != 0)
        {
            return -3;
        }
        return 0;
    }

    int Mysql::Ping() {
        if(mysql_ping(&m_pProp->connection) != 0)
        {
            FreeResult();
            mysql_close(&m_pProp->connection);
            m_pProp->bConnected = false;
            return -1;
        }
        return 0;
    }

    int Mysql:: Execute(const char* szSqlString)
    {
        Connect();
        if(mysql_ping(&m_pProp->connection) != 0)
        {
            FreeResult();
            mysql_close(&m_pProp->connection);
            m_pProp->bConnected = false;
            if(0 != Connect()) {
                m_pProp->bConnected = false;
                m_pProp->sErrMsg = string("mysql connection broken:%s") + string(mysql_error(&m_pProp->connection));
                m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
                LOG_ERROR << m_pProp->sErrMsg;
                return -1;
            }
        }
        //int size = strlen(szSqlString);
        if (MysqlQuery(&m_pProp->connection, szSqlString) != 0)
        {
            m_pProp->sErrMsg = string("mysql query failed:%s") + string(mysql_error(&m_pProp->connection));
            m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
            LOG_ERROR << m_pProp->sErrMsg;
            return -2;
        }
        return 0;
    }

    int Mysql:: ExecuteWithReconnect(const char* szSqlString)
    {
        //int size = strlen(szSqlString);
        if (MysqlQuery(&m_pProp->connection, szSqlString) != 0)
        {
            m_pProp->sErrMsg = string("mysql query failed:") + string(mysql_error(&m_pProp->connection));
            m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
            LOG_ERROR << m_pProp->sErrMsg;
            return -2;
        }
        return 0;
    }


    int Mysql:: FreeMem()
    {
        return FreeResult();
    }

    int Mysql:: FreeResult()
    {
        if (m_pProp->result != NULL)
            mysql_free_result (m_pProp->result);
        m_pProp->iField = 0;
        m_pProp->iRows = 0;
        if (m_pProp->bFieldIndexInitialized)
        {
            m_pProp->FieldIndex.erase(m_pProp->FieldIndex.begin(), m_pProp->FieldIndex.end());
            m_pProp->bFieldIndexInitialized = false;
        }
        m_pProp->result = NULL ;
        m_pProp->iRowNum = -1;
        m_pProp->row = NULL;
        return 0;
    }

    int Mysql:: StoreResult()
    {
        FreeResult();
        m_pProp->result = mysql_store_result (&m_pProp->connection);
        if (m_pProp->result == NULL)
        {
            m_pProp->sErrMsg = string("mysql StoreResult error: ") + string(mysql_error(&m_pProp->connection));
            m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
            LOG_ERROR << m_pProp->sErrMsg;
            return -1;
        }
        m_pProp->iField = mysql_num_fields (m_pProp->result);
        m_pProp->iRows =  mysql_num_rows(m_pProp->result);
        return 0;
    }

    unsigned int Mysql:: GetRecordCount()
    {
        return m_pProp->iRows;
    }

    unsigned int Mysql:: GetFieldCount()
    {
        return m_pProp->iField;
    }

    int Mysql:: InitFieldName()
    {
        if ((!m_pProp->bFieldIndexInitialized) && (m_pProp->result!=NULL))
        {
            int i;
            MYSQL_FIELD *field;
            int RowCount   =   mysql_num_fields(m_pProp->result);
            m_pProp->FieldNameList.resize(RowCount);
            for(i=0;i<RowCount;i++)
            {
                field = mysql_fetch_field(m_pProp->result);
                m_pProp->FieldIndex[field->name] = i;
                m_pProp->FieldNameList[i] = field->name;
            }
            m_pProp->bFieldIndexInitialized = true;
        }
        return 0;
    }

/*
 * query result
*/
    unsigned int Mysql:: GetAffectedRows()
    {
        my_ulonglong iNumRows;

        iNumRows = mysql_affected_rows(&m_pProp->connection);
        return (unsigned int)iNumRows;
    }

    const char * Mysql:: GetField( int iRow, unsigned int iField )
    {
        if(iRow<0 || (unsigned int)iRow>=m_pProp->iRows || m_pProp->result==NULL)
            return m_pProp->szNULL;
        else
        {
            if(iRow == m_pProp->iRowNum && NULL != m_pProp->row) {
                char* p = m_pProp->row[iField];
                if(p == NULL)
                    return m_pProp->szNULL;
                return p;
            }

            if(-1 == m_pProp->iRowNum || iRow != m_pProp->iRowNum + 1) {
                mysql_data_seek(m_pProp->result, iRow);
            }

            m_pProp->iRowNum = iRow;
            m_pProp->row = mysql_fetch_row(m_pProp->result);
            char* p = m_pProp->row[iField];
            if(p == NULL)
                return m_pProp->szNULL;
            return p;
        }
    }

    int Mysql:: MysqlNextResult()
    {
        return mysql_next_result(&m_pProp->connection);
    }

    int Mysql:: MysqlStoreResult()
    {
        return StoreResult();
    }

    MYSQL_ROW Mysql:: GetNextRow()
    {
        if(m_pProp->result==NULL)
            return NULL;
        else
        {
            m_pProp->row = mysql_fetch_row(m_pProp->result);
            if(NULL == m_pProp->row) {
                m_pProp->iRowNum = -1;
            } else {
                m_pProp->iRowNum++;
            }
            return m_pProp->row;
        }
    }

    const char * Mysql:: GetField( int iRow, const char * szFieldName )
    {
        InitFieldName();
        if(szFieldName == NULL)
            return m_pProp->szNULL;
        if (m_pProp->FieldIndex.find(szFieldName) == m_pProp->FieldIndex.end())
            return m_pProp->szNULL;
        return GetField(iRow, m_pProp->FieldIndex[szFieldName]);
    }

    const char * Mysql:: GetField( int iRow, unsigned int iField, unsigned long& iLen )
    {
        if(iRow<0 || (unsigned int)iRow>=m_pProp->iRows || m_pProp->result==NULL) {
            iLen = 0;
            return m_pProp->szNULL;
        }
        else
        {
            unsigned long * lengths = NULL;

            if(iRow == m_pProp->iRowNum && NULL != m_pProp->row) {
                char* p = m_pProp->row[iField];
                if(p == NULL) {
                    iLen = 0;
                    return m_pProp->szNULL;
                }

                lengths = mysql_fetch_lengths(m_pProp->result);
                if(NULL == lengths) {
                    m_pProp->sErrMsg = string("mysql_fetch_lengths failed:") + string(mysql_error(&m_pProp->connection));
                    m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
                    LOG_ERROR << m_pProp->sErrMsg;
                    iLen = 0;
                    return m_pProp->szNULL;
                }

                iLen = lengths[iField];
                return p;
            }

            if(-1 == m_pProp->iRowNum || iRow != m_pProp->iRowNum + 1) {
                mysql_data_seek(m_pProp->result, iRow);
            }

            m_pProp->iRowNum = iRow;

            m_pProp->row = mysql_fetch_row(m_pProp->result);

            lengths = mysql_fetch_lengths(m_pProp->result);
            if(NULL == lengths) {
                m_pProp->sErrMsg = string("mysql_fetch_lengths failed:") + string(mysql_error(&m_pProp->connection));
                m_pProp->iErrCode = mysql_errno(&m_pProp->connection);
                LOG_ERROR << m_pProp->sErrMsg;
                iLen = 0;
                return m_pProp->szNULL;
            }

            char* p = m_pProp->row[iField];
            if(p == NULL) {
                iLen = 0;
                return m_pProp->szNULL;
            }
            iLen = lengths[iField];
            return p;
        }
    }

    const char * Mysql:: GetField( int iRow, const char * szFieldName, unsigned long& iLen )
    {
        InitFieldName();
        if(szFieldName == NULL)
            return m_pProp->szNULL;
        if (m_pProp->FieldIndex.find(szFieldName) == m_pProp->FieldIndex.end()) {
            iLen = 0;
            return m_pProp->szNULL;
        }
        return GetField(iRow, m_pProp->FieldIndex[szFieldName], iLen);
    }

    int Mysql:: GetFieldNameList( std::vector<std::string> * pVec )
    {
        InitFieldName();
        if( pVec == NULL )
            return -1;
        *pVec = m_pProp->FieldNameList;
        return 0;
    }


    void Mysql::Close() {
        FreeResult();
        mysql_close(&m_pProp->connection);
        m_pProp->bConnected = false;
    }

    uint32_t Mysql::GetInsertId()
    {
        return mysql_insert_id(&m_pProp->connection);
    }

}
