#if !defined(RESIP_CERTIFICATE_AUTHENTICATOR_HXX)
#define RESIP_CERTIFICATE_AUTHENTICATOR_HXX 

#include <set>

#include "rutil/Data.hxx"
#include "repro/Processor.hxx"
#include "repro/Dispatcher.hxx"
#include "repro/ProxyConfig.hxx"

namespace resip
{
   class SipStack;
}

namespace repro
{
  class CertificateAuthenticator : public Processor
  {
    public:
      CertificateAuthenticator(ProxyConfig& config, resip::SipStack* stack, std::set<resip::Data>& trustedPeers);
      ~CertificateAuthenticator();

      virtual processor_action_t process(RequestContext &);
      virtual void dump(EncodeStream &os) const;

    private:
      bool authorizedForThisIdentity(const std::list<resip::Data>& peerNames, resip::Uri &fromUri);

      std::set<resip::Data> mTrustedPeers;
  };
  
}
#endif

/* ====================================================================
 * BSD License
 *
 * Copyright (c) 2012 Daniel Pocock  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 * OR ANY CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * ====================================================================
 *
 */

