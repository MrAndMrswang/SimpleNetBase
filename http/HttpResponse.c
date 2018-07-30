#include "HttpResponse.h"
#include "../cpp11netbase/Buffer.h"
#include <stdio.h>

#include <fstream>//new
#include <iostream>//new

void HttpResponse::appendToBuffer(Buffer* output) const
{
	char buf[32];
	snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
	output->append(buf);
	output->append(statusMessage_);
	output->append("\r\n");

	if (closeConnection_)
	{
		output->append("Connection: close\r\n");
	}
	else
	{
		snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
		output->append(buf);
		output->append("Connection: Keep-Alive\r\n");
	}

	for (std::map<string, string>::const_iterator it = headers_.begin();
			it != headers_.end();++it)
	{
		output->append(it->first);
		output->append(": ");
		output->append(it->second);
		output->append("\r\n");
	}

	output->append("\r\n");
	
	output->append(body_);
	//*************
	/*	std::ofstream outfile;
		outfile.open("../ResponseFile.txt",std::ios::app);
		if(outfile.is_open())
		{
			string s = output->toStringPiece().as_string();
			outfile<<s<<std::endl;
			outfile.close();
		}
	//*************
	*/
}

