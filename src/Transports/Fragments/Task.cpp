//***************************************************************************
// Copyright 2007-2014 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Universidade do Porto. For licensing   *
// terms, conditions, and further information contact lsts@fe.up.pt.        *
//                                                                          *
// European Union Public Licence - EUPL v.1.1 Usage                         *
// Alternatively, this file may be used under the terms of the EUPL,        *
// Version 1.1 only (the "Licence"), appearing in the file LICENCE.md       *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://www.lsts.pt/dune/licence.                                        *
//***************************************************************************
// Author: Jose Pinto                                                       *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

#include "IncomingMessage.hpp"

namespace Transports
{
  namespace Fragments
  {
    using DUNE_NAMESPACES;

    struct Arguments
    {
      // Reception timeout.
      float max_age_secs;
    };

    struct Task : public DUNE::Tasks::Task
    {
      std::map<uint32_t, IncomingMessage> m_incoming;
      Time::Counter<float> m_gc_counter;
      Arguments m_args;

      Task(const std::string& name, Tasks::Context& ctx):
        DUNE::Tasks::Task(name, ctx)
      {
        param("Reception timeout", m_args.max_age_secs)
        .defaultValue("1800")
        .description("Maximum amount of seconds to wait for missing fragments in incoming messages");

        bind<IMC::MessagePart>(this);
        m_gc_counter.setTop(120);
      }

      void
      onResourceRelease(void)
      {
        m_incoming.clear();
      }

      void
      consume(const IMC::MessagePart* msg)
      {
        int hash = (msg->uid << 16) | msg->getSource();

        if (m_incoming.find(hash) == m_incoming.end())
        {
          IncomingMessage incMsg;
          incMsg.setParentTask(this);
          m_incoming[hash] = incMsg;
        }

        debug("Incoming message fragment (%d still missing)",
              m_incoming[hash].getFragmentsMissing());

        IMC::Message * res = m_incoming[hash].setFragment(msg);
        if (res != NULL)
        {
          dispatch(res);
          m_incoming.erase(hash);
        }
      }

      void
      messageRipper(void)
      {
        debug("ripping old messages");
        std::map<uint32_t, IncomingMessage>::iterator it;

        for (it = m_incoming.begin(); it != m_incoming.end(); it++)
        {
          IncomingMessage msg = (*it).second;
          if (msg.getAge() > m_args.max_age_secs)
          {
            // message has died of natural causes...
            war("Removed incoming message from memory (%d fragments were still missing).",
                msg.getFragmentsMissing());
            m_incoming.erase(it);
          }
        }
      }

      void
      onMain(void)
      {
        while (!stopping())
        {
          waitForMessages(1.0);

          if (m_gc_counter.overflow())
          {
            messageRipper();
            m_gc_counter.reset();
          }
        }
      }
    };
  }
}

DUNE_TASK
