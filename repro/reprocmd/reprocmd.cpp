// testXmlRpc.cpp : Defines the entry point for the console application.
//

#include <rutil/DnsUtil.hxx>
#include <rutil/BaseException.hxx>
#include <rutil/XMLCursor.hxx>
#include <rutil/WinLeakCheck.hxx>

using namespace resip;
using namespace std;

void sleepSeconds(unsigned int seconds)
{
#ifdef WIN32
   Sleep(seconds*1000);
#else
   sleep(seconds);
#endif
}

int 
main (int argc, char** argv)
{
#ifdef WIN32
   initNetwork();
#endif

   int sd, rc;
   struct sockaddr_in localAddr, servAddr;
   struct hostent *h;

   if(argc < 4) 
   {
      cerr << "usage: " << argv[0] <<" <server> <port> <command> [<parm>=<value>]" << endl;
      cerr << "  Valid Commands are:" << endl;
      cerr << "  GetStackInfo - retrieves low level information about the stack state" << endl;
      cerr << "  GetStackStats - retrieves a dump of the stack statistics" << endl;
      cerr << "  ResetStackStats - resets all cummulative stack statistics to zero" << endl;
      cerr << "  LogDnsCache - causes the DNS cache contents to be written to the resip logs" << endl;
      cerr << "  ClearDnsCache - emptys the stacks DNS cache" << endl;
      cerr << "  GetDnsCache - retrieves the DNS cache contents" << endl;
      cerr << "  GetCongestionStats - retrieves the stacks congestion manager stats and state" << endl;
      cerr << "  SetCongestionTolerance metric=<SIZE|WAIT_TIME|TIME_DEPTH> maxTolerance=<value>" << endl;
      cerr << "                         [fifoDescription=<desc>] - sets congestion tolerances" << endl;
      exit(1);
   }

   h = gethostbyname(argv[1]);
   if(h==0) 
   {
      cerr << "unknown host " << argv[1] << endl;
      exit(1);
   }

   servAddr.sin_family = h->h_addrtype;
   memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
   servAddr.sin_port = htons(atoi(argv[2]));

   // Create TCP Socket
   sd = (int)socket(AF_INET, SOCK_STREAM, 0);
   if(sd < 0) 
   {
      cerr << "cannot open socket" << endl;
      exit(1);
   }

   // bind to any local interface/port
   localAddr.sin_family = AF_INET;
   localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   localAddr.sin_port = 0;

   rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
   if(rc < 0) 
   {
      cerr <<"error binding locally" << endl;
      exit(1);
   }

   // Connect to server
   rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
   if(rc < 0) 
   {
      cerr << "error connecting" << endl;
      exit(1);
   }

   Data request(1024, Data::Preallocate);
   request += "<";
   request += argv[3];
   request += ">\r\n  <Request>\r\n";
   for(int i = 4; i < argc; i++)
   {
      Data parm;
      Data value;
      Data arg(argv[i]);
      ParseBuffer pb(arg);
      const char *anchor = pb.position();
      pb.skipToChar('=');
      if(!pb.eof())
      {
         pb.data(parm, anchor);
         pb.skipChar();
         anchor = pb.position();
         pb.skipToEnd();
         pb.data(value, anchor);
      }
      request += "    <";
      request += parm;
      request += ">";
      request += value;
      request += "</";
      request += parm;
      request += ">\r\n";
   }
   request += "  </Request>\r\n";
   request += "</";
   request += argv[3];
   request += ">\r\n";

   //cout << "Sending:\r\n" << request << endl;

   rc = send(sd, request.c_str(), request.size(), 0);
   if(rc < 0) 
   {
      cerr << "error sending request on socket." << endl;
      closeSocket(sd);
      exit(1);
   }

   char readBuffer[8000];
   while(rc > 0)
   {
      rc = recv(sd, (char*)&readBuffer, sizeof(readBuffer), 0);
      if(rc < 0) 
      {
         cerr << "error receiving response from socket." << endl;
         closeSocket(sd);
         exit(1);
      }

      if(rc > 0)
      {
         Data response(Data::Borrow, (const char*)&readBuffer, rc);
         //cout << "Received response: \r\n" << response.xmlCharDataDecode() << endl;

         ParseBuffer pb(response);
         XMLCursor xml(pb);
         bool responseOK = false;
         if(xml.firstChild() && xml.nextSibling() && xml.firstChild())  // Move to Response node
         {
            while(true)
            {
               if(isEqualNoCase(xml.getTag(), "Result"))
               {
                  unsigned int code=0;
                  Data text;
                  XMLCursor::AttributeMap::const_iterator it = xml.getAttributes().find("Code");
                  if(it != xml.getAttributes().end())
                  {
                     code = it->second.convertUnsignedLong();
                  }
                  if(xml.firstChild())
                  {
                     text = xml.getValue();
                     xml.parent();
                  }
                  if(code >= 200 && code < 300)
                  {
                     // Success
                     cout << text << endl;
                  }
                  else
                  {
                     cout << "Error " << code << " processing request: " << text << endl;
                  }
                  responseOK = true;
               }
               else if(isEqualNoCase(xml.getTag(), "Data"))
               {
                  if(xml.firstChild())
                  {
                     cout << xml.getValue().xmlCharDataDecode() << endl;
                     xml.parent();
                  }
               }
               if(!xml.nextSibling())
               {
                  // break on no more sibilings
                  break;
               }
            }
         }
         if(!responseOK)
         {
            cout << "Unable to parse response:" << endl << response << endl;
         }

         closeSocket(sd); 
         break;
      }
   }
}
