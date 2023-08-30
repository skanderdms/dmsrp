/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2023
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 * Authors: Mohamed Skander DAAS <daas.skander@umc.edu.dz>, written after AODV by Pavel Boyko <boyko@iitp.ru>
 */
#include "dmsrp-helper.h"
#include "ns3/dmsrp-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

DmsrpHelper::DmsrpHelper() : 
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::dmsrp::RoutingProtocol");
}

DmsrpHelper* 
DmsrpHelper::Copy (void) const 
{
  return new DmsrpHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
DmsrpHelper::Create (Ptr<Node> node) const
{
  Ptr<dmsrp::RoutingProtocol> agent = m_agentFactory.Create<dmsrp::RoutingProtocol> ();
  node->AggregateObject (agent);
  return agent;
}

void 
DmsrpHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
DmsrpHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<dmsrp::RoutingProtocol> dmsrp = DynamicCast<dmsrp::RoutingProtocol> (proto);
      if (dmsrp)
        {
          currentStream += dmsrp->AssignStreams (currentStream);
          continue;
        }
      // Dmsrp may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<dmsrp::RoutingProtocol> listDmsrp;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listDmsrp = DynamicCast<dmsrp::RoutingProtocol> (listProto);
              if (listDmsrp)
                {
                  currentStream += listDmsrp->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}

}
