//
// Created by lonnyliu(刘卓) on 2019-04-11.
//

#ifndef DAO_MYSQLUTIL_H
#define DAO_MYSQLUTIL_H

#include <string>
#include <map>
#include <vector>
#include <mysql.h>
#include <stdint.h>

namespace Comm{

/**
 @brief Mysql used to supply common mysql db access API.
 */
    class Mysql
    {


    public:
        /**
		@brief default constructor
		*/
        Mysql();

        Mysql(const bool bIsReconnect);
        /**
		 * @brief constructor
         * @param szHost indicates db host name or ip.
         * @param szUser indicates db account user name.
         * @param szPass indicates db account password .
         * @param szDB indicates db db .
         * @param iPort indicates db port .
         */
        Mysql(const char* szHost, const char* szUser, const char* szPass, const char* szDB, int iPort);

        //default destructor
        ~Mysql();

        /**
		 * @brief Used to establist db connection.
         * @param szHost indicates db host name or ip.
         * @param szUser indicates db account user name.
         * @param szPass indicates db account password .
         * @param szDB indicates db db .
         * @param iPort indicates db port .
         * @return 0 on success, -1 on failure.
         */
        int Connect(const char* szHost, const char* szUser, const char* szPass,const char* szDB, int iPort);

        /**
         * @brief Used to establist db connection.
		 * @return 0 on success, -1 on failure.
         */
        int Connect();

        bool IsConnected();
        int SetCharset(const char * sCharset);
        const char* CharacterSetName();

        /**
         * @brief SetOption(MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&iMySqlTimeout)
         */
        int SetOption(mysql_option option, const char *arg);

        /**
         * @brief SetServerOption(MYSQL_OPTION_MULTI_STATEMENTS_ON)
         */
        int SetServerOption(enum_mysql_set_option option);

        /**
         * @brief Used to execute db select sql query.
         * @param szSqlString indicates sql string which should include binary data.
         * @return 0 on scuesss <0 fail.
         */
        int  Query(const char* szSqlString);

        int MysqlQuery( MYSQL * ptMysql, const char * sSql );

        int  QueryWithoutReconnect(const char* szSqlString);

        /**
         * @brief Used to execute db insert/update/delete sql query.
         * @param szSqlString indicates sql string which should include binary data.
         * @return 0 on scuesss <0 fail.
         */
        int  Execute(const char* szSqlString);

        //warning!!!!!!
        //this is actually Execute Without Reconnect
        //the function name is wrong
        int  ExecuteWithReconnect(const char* szSqlString);

        /**
         * @brief Used to get the count of sql query result.
         * @return result count.
         */
        unsigned int GetRecordCount();

        /**
         * @brief Used to get the count of sql query result Field Count.
         * @return Field Count.
         */
        unsigned int GetFieldCount();

        /**
         * @brief Used to get query affected rows.
         * @return affected row count.
         */
        unsigned int GetAffectedRows();

        /**
          * @brief Used to strip abnormal character .
          * @param to indictes striped stirng.
          * @param from indicates original string.
          * @param len indicates from string length.
          * @return 0 on success,else fail.
          */
        int StripField(std::string& to, const char* from,unsigned long len);

        /**
         * @brief Used to get a field of a record index by iRow iField.
         * @return filed value.
         */
        const char * GetField( int iRow, unsigned int iField );
        /**
         * @brief Used to get a field of a record index by iRow szFieldName.
         * @return filed value.
         */
        const char * GetField( int iRow, const char * szFieldName );

        /**
         * @brief Used to get free memory allocated by Query.
         * @return 0 on scuesss -1 fail.
         */
        int FreeMem();

        const char* getErrMsg(void) const;

        int getErrCode(void) const;

        int Ping();
        void Close();

        int MysqlNextResult();
        int MysqlStoreResult();
        MYSQL_ROW GetNextRow( );

        /**
         * @brief Used to get a field of a record index by iRow iField.
         * @return filed value and field len
         */
        const char * GetField( int iRow, unsigned int iField, unsigned long& iLen );

        /**
         * @brief Used to get a field of a record index by iRow szFieldName.
         * @return filed value and field len
         */
        const char * GetField( int iRow, const char * szFieldName, unsigned long& iLen );

        int GetFieldNameList( std::vector<std::string> * pVec);

        uint32_t GetInsertId();

    private:
        void InitLibrary();
        /**
         * Used to store query result.
        * @return 0 onsuccess -1 on failure.
        */
        int StoreResult();
        int FreeResult();
        int InitFieldName();

    private:
        typedef std::map<std::string, int> STRING2INT;
        struct MysqlProp;

        MysqlProp* m_pProp;
    };

}





#endif //DAO_MYSQLUTIL_H
