#ifndef yrssf_core_webserver
#define yrssf_core_webserver
#define WEB_FLAG_DEF        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING|HANDLER_PARSE_POST_DATA)
#define WEB_FLAG_UPL        (lwan_handler_flags_t)(HANDLER_PARSE_COOKIES|HANDLER_PARSE_QUERY_STRING)
#include "client.hpp"
#include "httpd.hpp"
void ajax(yrssf::httpd::request * req){
    yrssf::httpd::forbidden(req->fd);
}
void webserverRun(){
    yrssf::httpd::addrule(ajax,"/ajax");
    yrssf::httpd::run(1215);
}
#endif