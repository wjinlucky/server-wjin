#include "log.h"


namespace sylar{

const char* LogLevel::ToString(LogLevel::Level level){
    switch(level){
#define XX(name) \
        case: LogLevel::name;\
            return #name;\
            break;
        
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
     default:
         return "UNKNOW";
    }
    return "UNKNOW";
}

class MessageFormatItem:public LogFormatter::FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<event->getContent();
       }
};

class LevelFormatItem:public LogFormatter::FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<LogLevel::ToString(level);
       }

};

//启动后的消耗时间
class ElapseFormatItem:public LogFormatter:: FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<event->getElapse();
       }
};

class NameFormatItem:public LogFormatter:: FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<event->getElapse();
       }
};

class ThreadIdFormatItem:public LogFormatter:: FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<event->getThreadId();
       }
};

class FiberIdFormatItem:public LogFormatter:: FormatItem{
public:
       void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
          os<<event->getFiberId();
       }
};

class DateTimeFormatItem:public LogFormatter:: FormatItem{
public:
    DateTimeFormatItem(const std::string& format="%Y:%m:%d %H:%M;%S")
       :m_format(format){

       }
    void format(std::ostream& os,logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) {
        os<<event->getFiberId();
    }
private:
   std::string m_format;
};




// const char * m_file=nullptr;//文件名
//    int32_t m_line=0;//行号
//    uint32_t m_elapse=0;//程序启动开始到现在的毫秒数
//    u_int32_t m_threadId=0;//线程id
//    uint32_t m_fiberId=0;//协程id
//    u_int64_t m_time=0;//时间戳
//    std::string m_content;//

// }


logger::logger(const std::string& name)
   :m_name(name){

}

void logger::addAppender(LogAppender::ptr appender){
    m_appender.push_back(appender);
}

void logger::delAppender(LogAppender::ptr appender){
    for(auto it=m_appender.begin();it!=m_appender.end();++it){
        if(*it==appender){
            m_appender.erase(it);
            break;
        }
    }
}


void logger::Log(LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
        for(auto& i : m_appender){
            i->Log(level,event);
        }
    }

}



void logger::debug(LogEvent::ptr event){
    Log(LogLevel::DEBUG,event);

}
void logger::info(LogEvent::ptr event){
    Log(LogLevel::INFO,event);

}
void logger::warn(LogEvent::ptr event){
    Log(LogLevel::WARN,event);

}
void logger::error(LogEvent::ptr event){
    Log(LogLevel::ERROR,event);

}
void logger::fatal(LogEvent::ptr event){
    Log(LogLevel::FATAL,event);

}

FileLogAppender::FileLogAppender(const std::string& filename)
   :m_filestream(filename){
}

bool FileLogAppender::reopen(){
    if(m_filestream){
        m_filestream.close();
    }
    m_filestream.open(m_filestream);
    return !!m_filestream;
}

void FileLogAppender::Log(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level){
        std::cout<<m_formatter.format(logger,level,event);
    }

}
void StdoutLogAppender::(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event){
    if(level>m_level){
        std::cout<<m_formatter.format(logger,level,event);
    }
}


LogFormatter::LogFormatter(const std::string& pattern)
   :m_pattern(pattern){

}

std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level, std::string& pattern){
    std::stringstream ss;
    for(auto& i:m_items){
        i->format(ss,logger,level,event);
    }
    return ss.str();
}

//%xxx %xxx{xxx}    %%
void LogFormatter::init(){
    //str,format,type
    std::vector<std::tuple<std::string,std::string,int>> vec;
    std::string nstr;
    for(size_t i=0;i<m_pattern.size();++i){
        if(m_pattern[i]!='%'){
            nstr.append(1,m_pattern[i]);
            continue;
        }

        if((i+1)<m_pattern.size()){
            if(m_pattern[i+1]=='%'){
                nstr.append(1,'%');
                continue;
            }
        }

        size_t n=i+1;
        int fmt_status=0;
        size_t fmt_begin=0;

        std::string str;
        std::string fmt;
        while(n<m_pattern.size()){
            if(isspace(m_pattern[n])){
                break;
            }
            if(fmt_status==0){
                if(m_pattern[n]=='{'){
                    str=m_pattern.substr(i+1,n-i-1);
                    fmt_status=1;//解析格式
                    fmt_begin=n;
                    ++n;
                    continue；
                }
            }
            if(fmt_status==1){
                if(m_pattern[n]=='}'){
                    fmt=m_pattern.substr(fmt_begin+1，n-fmt_begin-1);
                    fmt_status=2;
                    break;

                }
            }
        }
        //正常
        if(fmt_status==0){
            if(!nstr.empty()){
                vec.push_back(std::make_pair(nstr,"",0));
            }
            str=m_pattern.substr(i+1,n-i-1);
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n;
        }
        //错误
        else if(fmt_status==1){
            std::cout<<"pattern parse error:"<<m_pattern<<"-"<<m_pattern.substr(i)<<std::endl;
            vec.push_back(std::make_tuple('<<pattern_error>>',fmt,0));
        }
        else if(fmt_status==2){
            if(!nstr.empty()){
                vec.push_back(std::make_pair(nstr,"",0));
            }
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n;
        }
    }
    if(!nstr.empty()){
         vec.push_back(std::make_pair(nstr,"",0));
    }

    static std::map<std::string,std::function<FormatItem::ptr(const std::string& str)>,FormatItem::ptr> s_format_items={
#define XX(str,C)\
        {str,[](const std::string& fmt){return FormatItem::ptr(new C(fmt));}
        XX(m,MessageFormatItem),
        XX(p,LevelFormatItem),
        XX(r,ElapseFormatItem),
        XX(c,ThreadIdFormatItem),
        XX()
#undef XX
    };

    //%m --消息体
    //%p -- level
    //%c --启动后时间
    //%c --日志名称
    //%t --线程id
    //%n --回车换行
    //%d --时间
    //%f --文件名
    //%l --行号

}



