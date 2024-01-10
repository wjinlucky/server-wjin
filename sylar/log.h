#ifndef _SYLAR_LOG_H_
#define _SYLAR_LOG_H_

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<sstream>
#include<fstream>
#include <map>
#include <vector>
#include <iostream>

namespace sylar{

class Logger;


//日志事件
class LogEvent{
public:
   typedef std::shared_ptr<LogEvent> ptr;
   LogEvent();

   const char* getFile() const {return m_file;}
   int32_t getline() const {return m_line;}
   int32_t getElapse() const {return m_elapse;}
   int32_t getThreadId() const {return m_threadId;}
   int32_t getFiberId() const {return m_fiberId;}
   int32_t getTime() const{return m_time;}
   const std::string& getContent() const {return m_content;}
private:
   const char * m_file=nullptr;//文件名
   int32_t m_line=0;//行号
   uint32_t m_elapse=0;//程序启动开始到现在的毫秒数
   u_int32_t m_threadId=0;//线程id
   uint32_t m_fiberId=0;//协程id
   u_int64_t m_time=0;//时间戳
   std::string m_content;//
};

//日志级别
class LogLevel{
public:
   enum Level{
       UNKNOW=0,
       DEBUG=1,
       INFO=2,
       WARN=3,
       ERROR=4,
       FATAL=5,   
   };
   static const char* ToString(LogLevel::Level level);
           
};
//日志格式器
class LogFormatter{
public:
   typedef std::shared_ptr<LogFormatter> ptr;
   LogFormatter(const std::string& pattern);//根据pattern解析信息

   //%t %thread_id %m%n
   std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
public:
   class FormatItem{
    public:
       typedef std::shared_ptr<FormatItem> ptr;//智能指针
       FormatItem (const std::string& fmt=""){};
       virtual ~FormatItem(){}
       virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event)=0;
   };
   void init();
private:
   std::string m_pattern;
   std::vector<FormatItem::ptr>m_items;
   

};

//日志输出地
class LogAppender{
public:
   typedef std::shared_ptr<LogAppender> ptr;
   virtual ~LogAppender(){}
   virtual void Log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)=0;

   void setFormatter(LogFormatter::ptr val){m_formatter=val;}
   LogFormatter::ptr getFormatter() const {return m_formatter;}
protected:
   LogLevel::Level m_level;
   LogFormatter:: ptr m_formatter;//
};


//日志器
class logger{
public:
   typedef std::shared_ptr<logger> ptr;
   logger(const std::string& name="root");
   void Log(LogLevel::Level,LogEvent::ptr event);
   void debug(LogEvent::ptr event);
   void info(LogEvent::ptr event);
   void warn(LogEvent::ptr event);
   void error(LogEvent::ptr event);
   void fatal(LogEvent::ptr event);
   void addAppender(LogAppender::ptr appender);
   void delAppender(LogAppender::ptr appender);
   LogLevel::Level getLevel() const {return m_level;}
   void setLevel(LogLevel::Level level){m_level=level;}

   const std::string getName() const {return m_name;}

private:
   std::string m_name;//日志名称
   LogLevel::Level m_level;//日志级别
   
   std::list<LogAppender::ptr> m_appender;//Appender集合
};
//输出到控制台的Appener
class StdoutLogAppender:public LogAppender{
public:
   typedef std::shared_ptr<StdoutLogAppender>ptr;
   void Log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;
};

//输出到文件的Appener
class FileLogAppender:public LogAppender{
public:
   typedef std::shared_ptr<FileLogAppender>ptr;
   FileLogAppender(const std::string& filename);
   void Log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event) override;
   //文件打开成功、返回true
   bool reopen();
private:
   std::string m_name;
   std::ofstream m_filestream;
};

}

#endif