#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../cpp11netbase/EventLoop.h"

#include <iostream>
#include <map>

char favicon[555]="favicon";
bool benchmark = false;

// 实际的请求处理
void onRequest(const HttpRequest &req, HttpResponse *resp)
{
	std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
	if (!benchmark)
	{
		const std::map<string, string> &headers = req.headers();
		for (std::map<string, string>::const_iterator it = headers.begin();
			it != headers.end();
			++it)
		{
			std::cout << it->first << ": " << it->second << std::endl;
		}
	}

	if (req.path() == "/")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/html");
		resp->addHeader("Server", "Muduo");
		string now = Timestamp::now().toFormattedString();
		resp->setBody("<html><head><title>This is title</title></head><body><a href=\"http://baidu.com\">www.baidu.com</a><nav> <br><a href=\"/css/\">CSS</a><br><a href=\"/login\">LOGIN</a> <br><a href=\"/parper\">MY_Parper</a> <br><a href=\"/jquery/\">jQuery</a></nav><h1>Hello</h1>Now is " + now +"</body></html>");
	}
	else if (req.path() == "/favicon.ico")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("image/png");
		resp->setBody(string(favicon, sizeof favicon));
	}
	else if (req.path() == "/parper")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/html");
		resp->addHeader("Server", "Muduo");
		string body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>菜鸟教程(runoob.com)</title></head><body><h1>我的第一个标题</h1><p>我的第一个段落。</p></body></html>";
		resp->setBody(body);
	}
	else if(req.path() == "/login")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/html");
		resp->addHeader("Server","Wc");
		string body = "<body><form name=\"form_1\" action=\"/cgi-bin/post.cgi\" method=\"post\"><table align=\"center\"><tr><td align=\"center\" colspan=\"2\"></td></tr><tr><td align=\"right\">username</td><td><input type=\"text\" name=\"Username\"></td> </tr><tr><td align=\"right\">password</td><td><input type=\"password\" name=\"Password\"></td></tr><tr><td><input type=\"submit\" value=\login></td><td><input type=\"reset\" value=\"cancel\"></td></tr></table></form></body>";
		resp->setBody(body);
	}
	else
	{
		resp->setStatusCode(HttpResponse::k404NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);
	}
}

int main(int argc, char *argv[])
{
	int numThreads = 0;
	if (argc > 1)
	{
	benchmark = true;
	numThreads = atoi(argv[1]);
	}
	EventLoop loop;
	InetAddress ad("127.0.0.1",2000);
	HttpServer server(&loop, ad, "dummy");
	server.setHttpCallback(onRequest);
	server.setThreadNum(numThreads);
	server.start();
	loop.loop();
}

