#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <tuple>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <assert.h>

#include "mysql.h"

using namespace std;

enum class Destination
{
    carrier_id = 0,
    country_code = 1,
    areacode = 2,
    effect_date = 3,
    expire_date = 4,
    destination_id = 5
};

struct DestinationVal
{
    std::list<std::string> areacode;
    std::string effect_date;
    std::string expire_date;
    std::string destination_id;
};

std::unordered_map<std::string,std::vector<struct DestinationVal>> carrier_code_map;

void dealAreaCode(const std::string& area_code,std::list<std::string>& areacode_list)
{
    areacode_list.clear();
    std::vector<std::string> vs;
    boost::split(vs,area_code,boost::is_any_of(","));
    for(const auto &elem : vs )
    {
        if(std::string::npos != elem.find("-"))
        {
            std::vector<std::string> vsTmp;
            boost::split(vsTmp,elem,boost::is_any_of("-"));
            assert(vsTmp.size() == 2 && vsTmp.at(0).length() == vsTmp.at(1).length());
            assert(boost::all( vsTmp.at(0), boost::is_digit()) && boost::all( vsTmp.at(1), boost::is_digit()));
            long long iStart = stoll(vsTmp.at(0).c_str(),nullptr,10);
            long long iEnd   = stoll(vsTmp.at(1).c_str(),nullptr,10);
            for(long long i = iStart; i<=iEnd; i++)
            {
                areacode_list.emplace_back(std::to_string(i));
            }
        }
        else
        {
            areacode_list.emplace_back(elem);
        }
    }
}
void display()
{
    for(const auto& elem: carrier_code_map)
    {
        std::cout << elem.first << "：" << std::endl;
        for(const auto &elemDestination : elem.second)
        {
            std::cout <<"effect_date:"<< elemDestination.effect_date << "  "
            <<"expire_date:"<< elemDestination.expire_date << "  " 
            <<"destination_id:"<< elemDestination.destination_id << "  areacode:{";
            for(const auto &areacodeItem : elemDestination.areacode)
            {
                std::cout << areacodeItem << ",";
            }
            std::cout << " }" << std::endl;
        }
    }
}

void dumpToMap(const vector<tuple<std::string,std::string,std::string,std::string,std::string,std::string>>& vtp_carrier_code)
{
    std::string sAreaCode;
    for(const auto &elem : vtp_carrier_code)
    {        
        struct DestinationVal stDestinationVal;
        std::string sCarrierId = std::get<static_cast<int>(Destination::carrier_id)>(elem);
        std::string sCountryCode = std::get<static_cast<int>(Destination::country_code)>(elem);
        std::string sKey = sCarrierId.append(sCountryCode);
        std::replace_if(sKey.begin(), sKey.end(),[](char ch){return std::isspace(ch)||(ch=='\t')||(ch=='\n');},'\0');
        sAreaCode = std::get<static_cast<int>(Destination::areacode)>(elem);
        stDestinationVal.effect_date = std::get<static_cast<int>(Destination::effect_date)>(elem);  
        stDestinationVal.expire_date = std::get<static_cast<int>(Destination::expire_date)>(elem);  
        stDestinationVal.destination_id = std::get<static_cast<int>(Destination::destination_id)>(elem);  
        dealAreaCode(sAreaCode,stDestinationVal.areacode);
        auto search = carrier_code_map.find(sKey);
        if(search != carrier_code_map.end())
        {
            search->second.emplace_back(std::move(stDestinationVal));
        }
        else
        {
            std::vector<struct DestinationVal> v;
            v.emplace_back(stDestinationVal);
            carrier_code_map.insert(std::make_pair(sKey,v));
        }
        
    }

}

int main(int argc, char **argv)
{
    vector<tuple<std::string,std::string,std::string,std::string,std::string,std::string>> vtpCarrierCode;
    MYSQL mysql,*conn; 
    mysql_init(&mysql);
    if (!(conn = mysql_real_connect(&mysql,"122.51.224.238","root","password","test",3306,NULL,0)))
    {
        fprintf(stderr,"Couldn't connect to engine!\n%s\n\n",mysql_error(&mysql));
        perror("Couldn't connect to engine");
        exit(1);
    }

    std::string sQuerySql = "select carrier_id,country_code,areacode,effect_date,expire_date,destination_id from carrier_code LIMIT 10";
    if(mysql_query(conn,sQuerySql.c_str()))
    {
        fprintf(stderr,"Query failed (%s)\n",mysql_error(conn));
        exit(1);
    }

    MYSQL_RES       *queryRes;
    my_ulonglong    numRows;
    unsigned int    numFields;

    if (!(queryRes=mysql_store_result(conn)))
    {
        fprintf(stderr,"Couldn't get result from %s\n",mysql_error(conn));
        exit(1);
    }
    else
    {
        numRows = mysql_num_rows(queryRes);
        numFields = mysql_num_fields(queryRes);
    }
    // Print column headers
    MYSQL_FIELD    *Fields = mysql_fetch_fields(queryRes);
    for(int i=0; i<numFields; i++)
    {
        printf("%s\t",Fields[i].name);
    }
    printf("\n");
    // print query results
    MYSQL_ROW       mysqlRow;
    // row pointer in the result set
    while(mysqlRow = mysql_fetch_row(queryRes))
    {
        /*for(int i=0; i< numFields; i++)
        {
            printf("%s\t\t", mysqlRow[i] ? mysqlRow[i] : "NULL");  // Not NULL then print        
        }*/
        std::string sCarrierId = mysqlRow[static_cast<int>(Destination::carrier_id)] ? mysqlRow[static_cast<int>(Destination::carrier_id)] : "";
        std::string sCountryCode = mysqlRow[static_cast<int>(Destination::country_code)] ? mysqlRow[static_cast<int>(Destination::country_code)] : "";
        std::string sAreaCode = mysqlRow[static_cast<int>(Destination::areacode)] ? mysqlRow[static_cast<int>(Destination::areacode)] : "";
        std::string sEffectDate = mysqlRow[static_cast<int>(Destination::effect_date)] ? mysqlRow[static_cast<int>(Destination::effect_date)] : "";
        std::string sExpireDate = mysqlRow[static_cast<int>(Destination::expire_date)] ? mysqlRow[static_cast<int>(Destination::expire_date)] : "";
        std::string sDestinationId = mysqlRow[static_cast<int>(Destination::destination_id)]?mysqlRow[static_cast<int>(Destination::destination_id)] : "";
        tuple<std::string,std::string,std::string,std::string,std::string,std::string> tpCarrierCode= std::make_tuple(\
            sCarrierId,sCountryCode,sAreaCode,sEffectDate,sExpireDate,sDestinationId);
        vtpCarrierCode.emplace_back(std::move(tpCarrierCode));       
        //printf("\n");
    }
    dumpToMap(vtpCarrierCode);
    display();
    if(queryRes)
    {
        mysql_free_result(queryRes);
        queryRes = NULL;
    }

    mysql_free_result(queryRes);

    mysql_close(conn);
    exit(0);
    return 0;					/* Keep some compilers happy */
}

/*
[root@VM_0_7_centos mysql]# g++ -o select_test select_test.cpp -I/usr/local/mysql/include  -L/usr/local/mysql/lib -lmysqlclient -lpthread -lm -ldl
[root@VM_0_7_centos mysql]# ./select_test
carrier_id      country_code    effect_date     expire_date     destination_id
57268                           20130701                20490101                1
57268                           20130701                20490101                2
57268           NULL            20140403                20170824                3
57268           NULL            20140403                20490101                4
57268           NULL            20140403                20170824                5
57268           NULL            20140403                20170824                6
57268           NULL            20140403                20170824                7
57268           NULL            20140403                20490101                8
57268                           20130701                20490101                9
57268                           20130701                20140403                10

*/