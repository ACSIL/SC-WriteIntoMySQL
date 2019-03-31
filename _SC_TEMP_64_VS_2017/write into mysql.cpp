#include "sierrachart.h"
#include <fstream>
#include <string>
#include <mysql.h>
#include <iostream>
#include <iomanip>
#include <fstream>

SCDLLName("WRITE INTO MYSQL")

void write_entry_details_into_db(SCStudyInterfaceRef sc)
{
    const char *server = sc.Input[0].GetString();
    const char *user = sc.Input[1].GetString();
    const char *password = sc.Input[2].GetString();
    const char *database = sc.Input[3].GetString();
    const char *table = sc.Input[4].GetString();

    MYSQL *conn;
    MYSQL_RES *res;

    //getting position details and converting all to strings for mysql queries
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);
    std::string symbol{ PositionData.Symbol }; //NQ
    std::string qty = std::to_string(PositionData.PositionQuantity); //-1 (long/short)
    std::string price = std::to_string(PositionData.AveragePrice); //6576:50

    //getting timestamp of position and converting them all to strings
    auto last_entry_dt = PositionData.LastEntryDateTime;
    int year_, month_, day_, hour_, minute_, second_;
    last_entry_dt.GetDateTimeYMDHMS(year_, month_, day_, hour_, minute_, second_);
    std::string year = std::to_string(year_);
    std::string month = std::to_string(month_);
    std::string day = std::to_string(day_);
    std::string hour = std::to_string(hour_);
    std::string minute = std::to_string(minute_);
    std::string second = std::to_string(second_);
    std::string entry_date = "'" + year + "-" + month + "-" + day + "'";
    std::string entry_time = "'" + hour + ":" + minute + ":" + second + "'";
    std::string entry_datetime = "'" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second + "'";

    //building sql CREATE TABLE query
    // -->  CREATE TABLE IF NOT EXISTS intraday (symbol varchar(100), qty int, entry_datetime datetime, exit_datetime datetime, entry_price float, profit_loss float, cummulative_pl float)
    std::string ct_s1 = "CREATE TABLE IF NOT EXISTS ";
    std::string ct_s2 = static_cast<std::string>(sc.Input[4].GetString());
    std::string ct_s3 = " (symbol varchar(100), qty int, entry_datetime datetime, exit_datetime datetime, entry_price float, profit_loss float, cummulative_pl float)";
    ct_s1 += ct_s2 += ct_s3;
    const char * query_create_table = ct_s1.c_str();

    //building sql INSERT INTO query 
    std::string s0 = "INSERT INTO ";
    std::string s1 = static_cast<std::string>(sc.Input[4].GetString());
    std::string s2 = " (symbol, qty, entry_datetime, entry_price) VALUES ( ";
    std::string s3 = "'" + symbol + "'";
    std::string s4 = "," + qty + ",";
    std::string s5 = entry_datetime + ",";
    std::string s6 = price + ")";
    s0 += s1 += s2 += s3 += s4 += s5 += s6;
    const char * query_insert = s0.c_str();
    
                        s_UseTool ready_l;
                        ready_l.Clear();
                        ready_l.ChartNumber = sc.ChartNumber;
                        ready_l.DrawingType = DRAWING_TEXT;
                        ready_l.FontBackColor = RGB(0, 0, 0);
                        ready_l.FontSize = 7;
                        ready_l.FontBold = false;
                        ready_l.AddMethod = UTAM_ADD_OR_ADJUST;
                        ready_l.UseRelativeVerticalValues = 1;
                        ready_l.Color = RGB(255, 255, 255);
                        ready_l.Region = 0;
                        ready_l.Text.Format("insert: %s", query_insert);
                        ready_l.LineNumber = 2;
                        ready_l.BeginDateTime = 1;
                        ready_l.BeginValue = 90;
                        sc.UseTool(ready_l);
                                               
    //setting db connection
    conn = mysql_init(0);
    conn = mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
    res = mysql_store_result(conn);
    int query_state;

    if (conn)
    {
        SCString connected{ "Success connected - writing entry parameters" };
        sc.AddMessageToLog(connected, 1);

        query_state = mysql_query(conn, query_create_table);
        if (!query_state)
        {
            SCString create_table{ "Successful creating a table - writing entry parameters" };
            sc.AddMessageToLog(create_table, 1);
        }
        else
        {
            SCString create_table_err{ "Creating table failed - writing entry parameters" };
            sc.AddMessageToLog(create_table_err, 1);
        }

        query_state = mysql_query(conn, query_insert);
        if (!query_state)
        {
            SCString insert_into_table{ "Successful insert into the table - writing entry parameters" };
            sc.AddMessageToLog(insert_into_table, 1);
        }
        else
        {
            SCString insert_into_table_err{ "Inserting into table failed - writing entry parameters" };
            sc.AddMessageToLog(insert_into_table_err, 1);
        }
    }
    else
    {
        SCString connected{ "ERROR !! Could not connect to write entry details" };
        sc.AddMessageToLog(connected, 1);
    }
     
    mysql_free_result(res);
    mysql_close(conn);
}

void update_results(SCStudyInterfaceRef sc)
{
    const char *server = sc.Input[0].GetString();
    const char *user = sc.Input[1].GetString();
    const char *password = sc.Input[2].GetString();
    const char *database = sc.Input[3].GetString();
    const char *table = sc.Input[4].GetString();

    MYSQL *conn;
    MYSQL_RES *res;

    //postion details
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);
    std::string symbol{ PositionData.Symbol }; 
    std::string profit_loss = std::to_string(PositionData.LastTradeProfitLoss); 

    //getting timestamps
    auto last_entry_dt{ PositionData.LastExitDateTime };
    int year_, month_, day_, hour_, minute_, second_;
    last_entry_dt.GetDateTimeYMDHMS(year_, month_, day_, hour_, minute_, second_);
    std::string year = std::to_string(year_);
    std::string month = std::to_string(month_);
    std::string day = std::to_string(day_);
    std::string hour = std::to_string(hour_);
    std::string minute = std::to_string(minute_);
    std::string second = std::to_string(second_);
    std::string exit_datetime = "'" + year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second + "'";

    //building the UPDATE query for profit_loss and exit time
    //" UPDATE tablename SET profit_loss = ....., runup = ......, drawdown = ........., exit_datetime where profit_loss is null and symbol = ..... ";
    std::string update = "UPDATE " + static_cast<std::string>(sc.Input[4].GetString()) + " SET profit_loss = " + profit_loss +
        // ", runup = " + runup +
         //", drawdown = " + drawdown +
        ", exit_datetime = " + exit_datetime +
        "  WHERE profit_loss is NULL AND symbol = " + "'" + symbol + "'" + ";";
    const char * query_update = update.c_str();

                        s_UseTool ready_l;
                        ready_l.Clear();
                        ready_l.ChartNumber = sc.ChartNumber;
                        ready_l.DrawingType = DRAWING_TEXT;
                        ready_l.FontBackColor = RGB(0, 0, 0);
                        ready_l.FontSize = 7;
                        ready_l.FontBold = false;
                        ready_l.AddMethod = UTAM_ADD_OR_ADJUST;
                        ready_l.UseRelativeVerticalValues = 1;
                        ready_l.Color = RGB(255, 255, 255);
                        ready_l.Region = 0;
                        ready_l.Text.Format("Update: %s", query_update);
                        ready_l.LineNumber = 2;
                        ready_l.BeginDateTime = 1;
                        ready_l.BeginValue = 90;
                        sc.UseTool(ready_l);

    conn = mysql_init(0);
    conn = mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
    res = mysql_store_result(conn);
    int query_state;
    if (conn)
    {
        SCString connected{ "Success connected - updating results" };
        sc.AddMessageToLog(connected, 1);

        query_state = mysql_query(conn, query_update);
        if (!query_state)
        {
            SCString insert_into_table{ "Successful updating results" };
            sc.AddMessageToLog(insert_into_table, 1);
        }
        else
        {
            SCString insert_into_table_err{ "Updating results failed" };
            sc.AddMessageToLog(insert_into_table_err, 1);
        }
    }
    else
    {
        SCString connected{ "ERROR !! Could not connect to update results" };
        sc.AddMessageToLog(connected, 1);
    }
    mysql_free_result(res);
    mysql_close(conn);
}

//void get_statistics_from_tradelog(SCStudyInterfaceRef sc)
//{
//    sc.MaintainTradeStatisticsAndTradesData = TRUE;
//
//    s_ACSTradeStatistics stats;
//    sc.GetTradeStatisticsForSymbol(0, 0, stats);
//
//    double closed_pl = stats.ClosedProfitLoss;
//    double closed_profit = stats.ClosedProfit;
//    double closed_loss = stats.ClosedLoss;
//    double profit_factor = stats.ProfitFactor;
//
//    double total_loosing_trades = stats.TotalLosingTrades;
//    double total_winning_trades = stats.TotalWinningTrades;
//     
//  }

void write_last_trade_from_tradelog_into_mysql(SCStudyInterfaceRef sc)
{
    SCDateTime open_dt, close_dt;
    std::string entry_qty, exit_qty, max_open_qty, entry_price, exit_price, open_pl, closed_pl, runup, drawdown, commission;

    s_ACSTrade trade;
    int last_trade = sc.GetTradeListSize() - 1;
    if (sc.GetTradeListEntry(last_trade, trade))
    {
        open_dt = trade.OpenDateTime;
        close_dt = trade.CloseDateTime;

        entry_qty = std::to_string(trade.EntryQuantity);
        exit_qty = std::to_string(trade.ExitQuantity);
        max_open_qty = std::to_string(trade.MaxOpenQuantity);
        entry_price = std::to_string(trade.AverageEntryPrice);
        exit_price = std::to_string(trade.AverageExitPrice);
        closed_pl = std::to_string(trade.ClosedProfitLoss);
        runup = std::to_string(trade.FlatToFlatMaximumOpenPositionProfit);
        drawdown = std::to_string(trade.FlatToFlatMaximumOpenPositionLoss);
        commission = std::to_string(trade.Commission);
    }
    //converting datetime to string 
    int oyear_, omonth_, oday_, ohour_, ominute_, osecond_;
    open_dt.GetDateTimeYMDHMS(oyear_, omonth_, oday_, ohour_, ominute_, osecond_);
    std::string open_year = std::to_string(oyear_);
    std::string open_month = std::to_string(omonth_);
    std::string open_day = std::to_string(oday_);
    std::string open_hour = std::to_string(ohour_);
    std::string open_minute = std::to_string(ominute_);
    std::string open_second = std::to_string(osecond_);
    std::string open_datetime = "'" + open_year + "-" + open_month + "-" + open_day + " " + open_hour + ":" + open_minute + ":" + open_second + "'";

    int cyear_, cmonth_, cday_, chour_, cminute_, csecond_;
    close_dt.GetDateTimeYMDHMS(cyear_, cmonth_, cday_, chour_, cminute_, csecond_);
    std::string close_year = std::to_string(cyear_);
    std::string close_month = std::to_string(cmonth_);
    std::string close_day = std::to_string(cday_);
    std::string close_hour = std::to_string(chour_);
    std::string close_minute = std::to_string(cminute_);
    std::string close_second = std::to_string(csecond_);
    std::string close_datetime = "'" + close_year + "-" + close_month + "-" + close_day + " " + close_hour + ":" + close_minute + ":" + close_second + "'";

    //convertig direction (+1/-1) to string
    std::string dir;
    trade.TradeType > 0 ? dir = "Long" : dir = "Short";

    //converting symbol to string
    std::string symbol = static_cast<std::string>(sc.GetChartSymbol(sc.ChartNumber));

    //building sql CREATE TABLE query
    std::string ct_s1 = "CREATE TABLE IF NOT EXISTS ";
    std::string ct_s2 = static_cast<std::string>(sc.Input[4].GetString()); //name of the table from input
    std::string ct_s3 = " (id INT AUTO_INCREMENT PRIMARY KEY,"
                        " symbol VARCHAR(20),"
                        " trade_type VARCHAR(10),"
                        " entry_datetime DATETIME,"
                        " exit_datetime DATETIME,"
                        " entry_price FLOAT,"
                        " exit_price FLOAT,"
                        " trade_qty FLOAT,"
                        " max_open_qty FLOAT,"
                        " profitloss FLOAT,"
                        " running_profitloss FLOAT,"
                        " runup FLOAT,"
                        " drawdown FLOAT,"
                        " commission FLOAT)";
    ct_s1 += ct_s2 += ct_s3;
    const char * query_create_table = ct_s1.c_str();
                                                                                                                                                           
    //building sql INSERT INTO query 
    std::string s0 = "INSERT INTO ";
    std::string s1 = static_cast<std::string>(sc.Input[4].GetString()); 
    std::string s2 = " (symbol, trade_type, entry_datetime, exit_datetime, entry_price, exit_price, trade_qty, max_open_qty, profitloss, runup, drawdown, commission) VALUES ( ";
    std::string s3 = "'" + symbol + "'";
    std::string s3a = ",";
    std::string s4 = "'" + dir + "'";
    std::string s4a = ",";
    std::string s5 = open_datetime + ",";
    std::string s6 = close_datetime + ",";
    std::string s7 = entry_price + ",";
    std::string s8 = exit_price + ",";
    std::string s9 = entry_qty + ",";
    std::string s10 = max_open_qty + ",";
    std::string s11 = closed_pl + ",";
    std::string s12 = runup + ",";
    std::string s13 = drawdown + ",";
    std::string s14 = commission + ")";
    s0 += s1 += s2 += s3 += s3a += s4 += s4a += s5 += s6 += s7 += s8 += s9 += s10 += s11 += s12 += s13 += s14;
    const char * query_insert = s0.c_str();

    //setting db connection                                                                             
    const char *server = sc.Input[0].GetString();
    const char *user = sc.Input[1].GetString();
    const char *password = sc.Input[2].GetString();
    const char *database = sc.Input[3].GetString();
    const char *table = sc.Input[4].GetString();

    MYSQL *conn;
    MYSQL_RES *res;

    conn = mysql_init(0);
    conn = mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
    res = mysql_store_result(conn);
    int query_state;

    if (conn)
    {
        SCString connected{ "Success connected - writing entry parameters" };
        sc.AddMessageToLog(connected, 1);

        query_state = mysql_query(conn, query_create_table);
        if (!query_state)
        {
            SCString create_table{ "Successful creating a table - writing entry parameters" };
            sc.AddMessageToLog(create_table, 1);
        }
        else
        {
            SCString create_table_err{ "Creating table failed - writing entry parameters" };
            sc.AddMessageToLog(create_table_err, 1);
        }

        query_state = mysql_query(conn, query_insert);
        if (!query_state)
        {
            SCString insert_into_table{ "Successful insert into the table - writing entry parameters" };
            sc.AddMessageToLog(insert_into_table, 1);
        }
        else
        {
            SCString insert_into_table_err{ "Inserting into table failed - writing entry parameters" };
            sc.AddMessageToLog(insert_into_table_err, 1);
        }
    }
    else
    {
        SCString connected{ "ERROR !! Could not connect to write entry details" };
        sc.AddMessageToLog(connected, 1);
    }

    mysql_free_result(res);
    mysql_close(conn);
}

void write_all_trades_from_tradelog_into_mysql(SCStudyInterfaceRef sc)
{
    //setting db connection                                                                             
    const char *server = sc.Input[0].GetString();
    const char *user = sc.Input[1].GetString();
    const char *password = sc.Input[2].GetString();
    const char *database = sc.Input[3].GetString();
    const char *table = sc.Input[4].GetString();

    MYSQL *conn;
    MYSQL_RES *res;

    conn = mysql_init(0);
    conn = mysql_real_connect(conn, server, user, password, database, 0, NULL, 0);
    res = mysql_store_result(conn);
    int query_state;

    //building sql CREATE TABLE query
    std::string ct_s1 = "CREATE TABLE IF NOT EXISTS ";
    std::string ct_s2 = static_cast<std::string>(sc.Input[4].GetString()); //name of the table from input
    std::string ct_s3 = " (id INT AUTO_INCREMENT PRIMARY KEY,"
        " symbol VARCHAR(20),"
        " trade_type VARCHAR(10),"
        " entry_datetime DATETIME,"
        " exit_datetime DATETIME,"
        " entry_price FLOAT,"
        " exit_price FLOAT,"
        " trade_qty FLOAT,"
        " max_open_qty FLOAT,"
        " profitloss FLOAT,"
        " running_profitloss FLOAT,"
        " runup FLOAT,"
        " drawdown FLOAT,"
        " commission FLOAT)";
    ct_s1 += ct_s2 += ct_s3;
    const char * query_create_table = ct_s1.c_str();

    SCDateTime open_dt, close_dt;
    std::string entry_qty, exit_qty, max_open_qty, entry_price, exit_price, open_pl, closed_pl, runup, drawdown, commission;
    std::string symbol = static_cast<std::string>(sc.GetChartSymbol(sc.ChartNumber));
    std::string dir;

    s_ACSTrade trade;
    std::vector <s_ACSTrade> vec_trades;
         
    //fill the vector with the trades´ details
    for (int i = 0; i < sc.GetTradeListSize(); ++i)
    {
        if (sc.GetTradeListEntry(i, trade)) 
            vec_trades.push_back(trade);
    }
    

    if (conn)
    {
        SCString connected{ "Success connected - writing entry parameters" };
        sc.AddMessageToLog(connected, 1);

        query_state = mysql_query(conn, query_create_table);
        if (!query_state)
        {
            SCString create_table{ "Successful creating a table - writing entry parameters" };
            sc.AddMessageToLog(create_table, 1);
        }
        else
        {
            SCString create_table_err{ "Creating table failed - writing entry parameters" };
            sc.AddMessageToLog(create_table_err, 1);
        }

        //iterate throught the vector and write each field into newly created mysql table
        for (unsigned int i = 0; i < vec_trades.size(); i++)
        {
            open_dt = vec_trades[i].OpenDateTime;
            close_dt = vec_trades[i].CloseDateTime;
            entry_qty = std::to_string(vec_trades[i].EntryQuantity);
            exit_qty = std::to_string(vec_trades[i].ExitQuantity);
            max_open_qty = std::to_string(vec_trades[i].MaxOpenQuantity);
            entry_price = std::to_string(vec_trades[i].AverageEntryPrice);
            exit_price = std::to_string(vec_trades[i].AverageExitPrice);
            closed_pl = std::to_string(vec_trades[i].ClosedProfitLoss);
            runup = std::to_string(vec_trades[i].FlatToFlatMaximumOpenPositionProfit);
            drawdown = std::to_string(vec_trades[i].FlatToFlatMaximumOpenPositionLoss);
            commission = std::to_string(vec_trades[i].Commission);

            vec_trades[i].TradeType > 0 ? dir = "Long" : dir = "Short";
            
            //converting datetime to string 
            int oyear_, omonth_, oday_, ohour_, ominute_, osecond_;
            open_dt.GetDateTimeYMDHMS(oyear_, omonth_, oday_, ohour_, ominute_, osecond_);
            std::string open_year = std::to_string(oyear_);
            std::string open_month = std::to_string(omonth_);
            std::string open_day = std::to_string(oday_);
            std::string open_hour = std::to_string(ohour_);
            std::string open_minute = std::to_string(ominute_);
            std::string open_second = std::to_string(osecond_);
            std::string open_datetime = "'" + open_year + "-" + open_month + "-" + open_day + " " + open_hour + ":" + open_minute + ":" + open_second + "'";
            int cyear_, cmonth_, cday_, chour_, cminute_, csecond_;
            close_dt.GetDateTimeYMDHMS(cyear_, cmonth_, cday_, chour_, cminute_, csecond_);
            std::string close_year = std::to_string(cyear_);
            std::string close_month = std::to_string(cmonth_);
            std::string close_day = std::to_string(cday_);
            std::string close_hour = std::to_string(chour_);
            std::string close_minute = std::to_string(cminute_);
            std::string close_second = std::to_string(csecond_);
            std::string close_datetime = "'" + close_year + "-" + close_month + "-" + close_day + " " + close_hour + ":" + close_minute + ":" + close_second + "'";

            //building sql INSERT INTO query 
            std::string s0 = "INSERT INTO ";
            std::string s1 = static_cast<std::string>(sc.Input[4].GetString());
            std::string s2 = " (symbol, trade_type, entry_datetime, exit_datetime, entry_price, exit_price, trade_qty, max_open_qty, profitloss, runup, drawdown, commission) VALUES ( ";
            std::string s3 = "'" + symbol + "'";
            std::string s3a = ",";
            std::string s4 = "'" + dir + "'";
            std::string s4a = ",";
            std::string s5 = open_datetime + ",";
            std::string s6 = close_datetime + ",";
            std::string s7 = entry_price + ",";
            std::string s8 = exit_price + ",";
            std::string s9 = entry_qty + ",";
            std::string s10 = max_open_qty + ",";
            std::string s11 = closed_pl + ",";
            std::string s12 = runup + ",";
            std::string s13 = drawdown + ",";
            std::string s14 = commission + ")";
            s0 += s1 += s2 += s3 += s3a += s4 += s4a += s5 += s6 += s7 += s8 += s9 += s10 += s11 += s12 += s13 += s14;
            const char * query_insert = s0.c_str();

            query_state = mysql_query(conn, query_insert); 
            
            if (!query_state)
            {
                SCString insert_into_table{ "Successful insert into the table - writing entry parameters" };
                sc.AddMessageToLog(insert_into_table, 1);
            }
            else
            {
                SCString insert_into_table_err{ "Inserting into table failed - writing entry parameters" };
                sc.AddMessageToLog(insert_into_table_err, 1);
            }
        }
    }
    else
    {
        SCString connected{ "ERROR !! Could not connect to write entry details" };
        sc.AddMessageToLog(connected, 1);
    }
    mysql_free_result(res);
    mysql_close(conn);
}


void testing_vector(SCStudyInterfaceRef sc)
{
    std::vector <s_ACSTrade> TradesList;

    s_ACSTrade TradeEntry;
    int Size = sc.GetTradeListSize();

    for (int Index = 0; Index < Size; Index++)
    {
        if (sc.GetTradeListEntry(Index, TradeEntry))
            TradesList.push_back(TradeEntry);
    }

    double ProfitLoss;
    double EntryPrice;

    for (unsigned int TradeIndex = 0; TradeIndex < TradesList.size(); TradeIndex++)
    {
        ProfitLoss = TradesList[TradeIndex].ClosedProfitLoss;
        EntryPrice = TradesList[TradeIndex].AverageEntryPrice;
        
        SCString log;
        log.Format("pl je %f, a vstupni cena je %f", ProfitLoss, EntryPrice);
        sc.AddMessageToLog(log, 0);

    }
   

    return;
}


//after opening a position, writes the defaults into mysql, after closing, updates result
SCSFExport scsf_LogIntoMySQL(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphRegion = 0;
        sc.GraphName = "General foo for logging into mysql";
        sc.GlobalDisplayStudyNameSubgraphNamesAndValues = 0;
        sc.FreeDLL = 1;
        sc.UpdateAlways = 1;
        sc.Input[0].Name = "Server";
        sc.Input[0].SetString("localhost");
        sc.Input[1].Name = "User";
        sc.Input[1].SetString("root");
        sc.Input[2].Name = "Password";
        sc.Input[2].SetString("password");
        sc.Input[3].Name = "Database";
        sc.Input[3].SetString("trading");
        sc.Input[4].Name = "Table";
        sc.Input[4].SetString("intraday");
        return;
    }

    int &previous_qt_perzist = sc.GetPersistentInt(0);
    int &already_logged_perzist = sc.GetPersistentInt(1);

    SCString log;
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);
    std::string symbol{ PositionData.Symbol };
    t_OrderQuantity32_64 qty{ PositionData.PositionQuantity };

    //writing into database
    if (previous_qt_perzist == 0 && PositionData.PositionQuantity != 0) 
    {
        previous_qt_perzist = 1;
    }
    if (previous_qt_perzist == 1 && PositionData.PositionQuantity == 0)
    {
        write_all_trades_from_tradelog_into_mysql(sc);
        //testing_vector(sc);
        previous_qt_perzist = 0;
    }


}