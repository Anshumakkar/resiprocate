#include "resiprocate/os/Log.hxx"
#include "resiprocate/os/Logger.hxx"
#include "resiprocate/Security.hxx"
#include "resiprocate/SipStack.hxx"
#include "resiprocate/StackThread.hxx"
#include "resiprocate/dum/InMemoryRegistrationDatabase.hxx"

#include "repro/Proxy.hxx"
#include "repro/RequestProcessorChain.hxx"
#include "repro/monkeys/RouteProcessor.hxx"
#include "repro/monkeys/DigestAuthenticator.hxx"
#include "repro/monkeys/LocationServer.hxx"
#include "repro/UserDb.hxx"
#include "repro/Registrar.hxx"
#include "repro/WebAdmin.hxx"
#include "repro/WebAdminThread.hxx"


#define RESIPROCATE_SUBSYSTEM Subsystem::REPRO

using namespace repro;
using namespace resip;
using namespace std;

int
main(int argc, char** argv)
{

/* Initialize a stack */
   Log::initialize(Log::Cout, Log::Info, argv[0]);
   Security security;
   SipStack stack(&security);
   stack.addTransport(UDP,5060);
   stack.addTransport(TCP,5060);
   stack.addTransport(TLS,5061);
   StackThread stackThread(stack);

   InMemoryRegistrationDatabase regData;

   /* Initialize a proxy */
   RequestProcessorChain requestProcessors;

   RequestProcessorChain* locators = new RequestProcessorChain();

   RouteProcessor* rp = new RouteProcessor();
   locators->addProcessor(std::auto_ptr<RequestProcessor>(rp));

   LocationServer* ls = new LocationServer(regData);
   locators->addProcessor(std::auto_ptr<RequestProcessor>(ls));

   requestProcessors.addProcessor(auto_ptr<RequestProcessor>(locators));

   DigestAuthenticator* da = new DigestAuthenticator();
   requestProcessors.addProcessor(std::auto_ptr<RequestProcessor>(da)); 
   

   UserDb userDb;
   WebAdmin admin(userDb);
   WebAdminThread adminThread(admin);
   
   Proxy proxy(stack, requestProcessors, userDb);

   /* Initialize a registrar */
   DialogUsageManager dum(stack);
   MasterProfile profile;
   Registrar registrar;

   profile.clearSupportedMethods();
   profile.addSupportedMethod(resip::REGISTER);

   dum.setServerRegistrationHandler(&registrar);
   dum.setRegistrationPersistenceManager(&regData);
   dum.setMasterProfile(&profile);

   /* Make it all go */
   stackThread.run();
   proxy.run();
   registrar.run();
   adminThread.run();
   
   registrar.join();
   proxy.join();
   stackThread.join();
   adminThread.join();
   
   // shutdown the stack now...
}
