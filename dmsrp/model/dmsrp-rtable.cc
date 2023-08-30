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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 DMSRP model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      DMSRP-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/DMSRP-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */

#include "dmsrp-rtable.h"
#include <algorithm>
#include <iomanip>
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DmsrpRoutingTable");

namespace dmsrp {


// DMS  RoutingTableEntryUp  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RoutingTableEntryUp::RoutingTableEntryUp (Ptr<NetDevice> dev, Ipv4Address dst,uint32_t seqNo,
                                      Ipv4InterfaceAddress iface, uint16_t hops, Ipv4Address nextHop, Time lifetime, float cumEnergy, float minSnr)
  : m_ackTimer (Timer::CANCEL_ON_DESTROY),
    m_seqNo (seqNo),
    m_hops (hops),
    m_lifeTime (lifetime + Simulator::Now ()),
    m_cumEnergy (cumEnergy),
    m_minSnr (minSnr),
    m_iface (iface)
{
  m_ipv4Route = Create<Ipv4Route> ();
  m_ipv4Route->SetDestination (dst);
  m_ipv4Route->SetGateway (nextHop);
  m_ipv4Route->SetSource (m_iface.GetLocal ());
  m_ipv4Route->SetOutputDevice (dev);
}


RoutingTableEntryUp::~RoutingTableEntryUp ()
{
}

void
RoutingTableEntryUp::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::ostream* os = stream->GetStream ();
  *os << m_ipv4Route->GetDestination () << "\t" << m_ipv4Route->GetGateway ()
      << "\t" << m_iface.GetLocal () << "\t";

  *os << "\t";
  *os << std::setiosflags (std::ios::fixed) <<
  std::setiosflags (std::ios::left) << std::setprecision (2) <<
  std::setw (14) << (m_lifeTime - Simulator::Now ()).GetSeconds ();
  *os << "\t" << m_hops <<"\t" << m_cumEnergy<<"\t" << m_minSnr<< "\n";
}


void
RoutingTableEntryUp::PrintInScreen () const
{

 //NS_LOG_UNCOND(m_ipv4Route->GetDestination () << "\t" << m_ipv4Route->GetGateway () << "\t" << m_iface.GetLocal ()<< "\t" /*<< str1*/<< "\t"<< std::setiosflags (std::ios::fixed) << std::setiosflags (std::ios::left) << std::setprecision (2) <<  std::setw (14) << (m_lifeTime - Simulator::Now ()).GetSeconds () <<"\t" << m_hops << "\t" << m_seqNo << "\t" << m_cumEnergy << "\t\t" << m_minSnr << "\n");

}

// end  RoutingTableEntryUp  DMS //////////////////////////////////////////////////////////////////////////////////////////



// DMS  RoutingTableEntryDown  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RoutingTableEntryDown::RoutingTableEntryDown (Ptr<NetDevice> dev, Ipv4Address dst,uint32_t seqNo,
                                      Ipv4InterfaceAddress iface, uint16_t hops, Ipv4Address nextHop, Time lifetime)
  : m_ackTimer (Timer::CANCEL_ON_DESTROY),
    m_seqNo (seqNo),
    m_hops (hops),
    m_lifeTime (lifetime + Simulator::Now ()),
    m_iface (iface)
{
  m_ipv4Route = Create<Ipv4Route> ();
  m_ipv4Route->SetDestination (dst);
  m_ipv4Route->SetGateway (nextHop);
  m_ipv4Route->SetSource (m_iface.GetLocal ());
  m_ipv4Route->SetOutputDevice (dev);
}


RoutingTableEntryDown::~RoutingTableEntryDown ()
{
}

void
RoutingTableEntryDown::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::ostream* os = stream->GetStream ();
  *os << m_ipv4Route->GetDestination () << "\t" << m_ipv4Route->GetGateway ()
      << "\t" << m_iface.GetLocal () << "\t";

  *os << "\t";
  *os << std::setiosflags (std::ios::fixed) <<
  std::setiosflags (std::ios::left) << std::setprecision (2) <<
  std::setw (14) << (m_lifeTime - Simulator::Now ()).GetSeconds ();
  *os << "\t" << m_hops << "\n";
}


void
RoutingTableEntryDown::PrintInScreen () const
{
 //NS_LOG_UNCOND(m_ipv4Route->GetDestination () << "\t" << m_ipv4Route->GetGateway () << "\t" << m_iface.GetLocal ()<< "\t" /*<< str1*/<< "\t"<< std::setiosflags (std::ios::fixed) << std::setiosflags (std::ios::left) << std::setprecision (2) <<  std::setw (14) << (m_lifeTime - Simulator::Now ()).GetSeconds () <<"\t" << m_hops << "\n" << m_seqNo << "\n");

}


// end  RoutingTableEntryDown  DMS //////////////////////////////////////////////////////////////////////////////////////////


/*
 The Routing Table RoutingTableUp
 */
//   RoutingTableUp DMS ////////////////////////////////////////////////////////////////////////////////////////////////////

RoutingTableUp::RoutingTableUp ()
{

}


bool
RoutingTableUp::IsEmpty ()     // DMS created by DMS
{
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      return true;
    }
 return false;

 
}


bool //DMS
RoutingTableUp::LookupTheRoute (RoutingTableEntryUp & rt)  //DMS (lookup the route that lead tothe sink) added by dms
{
//  NS_LOG_FUNCTION (this << id);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      NS_LOG_LOGIC ("Route not found; m_ipv4AddressEntry is empty");
      return false;
    }
  //std::map<Ipv4Address, RoutingTableEntryUp> i =
    rt = m_ipv4AddressEntry.begin ()->second;
  NS_LOG_LOGIC ("Route found");
  return true;
}

uint32_t
RoutingTableUp::GetMinHops ()
{
  NS_LOG_FUNCTION (this);
uint32_t minHops=10000;
  if (m_ipv4AddressEntry.empty ())
    {
      return minHops;
    }
          
          for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
                 m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
            {

              if (i->second.GetHop() < minHops)
                {
                      minHops= i->second.GetHop() ;
                }
               ++i;
            }
 return minHops;
}

bool
RoutingTableUp::LookupBestRoute (RoutingTableEntryUp & rt)   // DMS //All entries have the same number of hps in routingtableup, 
{
  NS_LOG_FUNCTION (this);
double maxCumEnergyValue=-1.0;
double maxMinSnrValue=-1.0;
Ipv4Address BestParentRoutAddress;
//uint16_t minHops=10000;
Time minLifetime=Seconds(100);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      return false;
    }

  switch (m_routingMode)
    {
    case BASIC_MODE:
      {
          rt = m_ipv4AddressEntry.begin ()->second;
          return true;
          break;
      }
    case MULTI_PARENT_MODE:
      {
         BestParentRoutAddress = m_ipv4AddressEntry.begin ()->second.GetNextHop ();
         minLifetime=m_ipv4AddressEntry.begin ()->second.GetLifeTime ();
          for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
                 m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
            {
                      if (i->second.GetLifeTime() > minLifetime)
                        {
                             minLifetime=Seconds(i->second.GetLifeTime()) ;
                             BestParentRoutAddress=i->second.GetNextHop ();
                        }
               ++i;
            }

          std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i = m_ipv4AddressEntry.find (BestParentRoutAddress);
          rt = i->second;
          return true;
          break;
      }
    case ENERGY_AWARE_MULTI_PARENT_MODE:
      {
          for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
                 m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
            {

                      if (i->second.GetCumEnergy() > maxCumEnergyValue)
                        {
                             maxCumEnergyValue=i->second.GetCumEnergy() ;
                             BestParentRoutAddress=i->second.GetNextHop ();
                        }
               ++i;
            }

          std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i = m_ipv4AddressEntry.find (BestParentRoutAddress);
          rt = i->second;
          return true;
          break;
      }
    case SNR_AWARE_MULTI_PARENT_MODE:
      {

          for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
                 m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
            {

                      if (i->second.GetMinSnr () > maxMinSnrValue)
                        {
                             maxMinSnrValue=i->second.GetMinSnr() ;
                             BestParentRoutAddress=i->second.GetNextHop ();
                        }

               ++i;
            }

          std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i = m_ipv4AddressEntry.find (BestParentRoutAddress);
          rt = i->second;
          return true;
          break;
        break;
      }    

    }
          return false;

}

bool //DMS
RoutingTableUp::GetNextNode (Ipv4Address & NextNodeAdr)  //get the IP adresse of the next node to keep the sink - DMS added by dms
{
//  NS_LOG_FUNCTION (this << id);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      NS_LOG_LOGIC ("Adresse not fount; m_ipv4AddressEntry is empty");
      return false;
    }

NextNodeAdr= (m_ipv4AddressEntry.begin ()->second).GetDestination ();

  NS_LOG_LOGIC ("Route found");
  return true;
}


bool
RoutingTableUp::LookupRoute (Ipv4Address id, RoutingTableEntryUp & rt)
{
  NS_LOG_FUNCTION (this << id);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found; m_ipv4AddressEntry is empty");
      return false;
    }
  std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i =
    m_ipv4AddressEntry.find (id);
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found");
      return false;
    }
  rt = i->second;
  NS_LOG_LOGIC ("Route to " << id << " found");
  return true;
}


bool
RoutingTableUp::AddRoute (RoutingTableEntryUp & rt)
{
  NS_LOG_FUNCTION (this);
  Purge ();
/*
if(m_routingMode==BASIC_MODE)
{
NS_LOG_UNCOND("Mooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooee is: BASIC_MODE" <<BASIC_MODE);
}

if(m_routingMode==MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooee is: MULTI_PARENT_MODE" <<MULTI_PARENT_MODE);
}

if(m_routingMode==ENERGY_AWARE_MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooee is: ENERGY_AWARE_MULTI_PARENT_MODE" <<ENERGY_AWARE_MULTI_PARENT_MODE);
}

if(m_routingMode==SNR_AWARE_MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooee is: SNR_AWARE_MULTI_PARENT_MODE" <<SNR_AWARE_MULTI_PARENT_MODE);
}

*/



if(m_routingMode==BASIC_MODE)
{
m_ipv4AddressEntry.erase (m_ipv4AddressEntry.begin(),m_ipv4AddressEntry.end());
}
else
{
 m_ipv4AddressEntry.erase (rt.GetNextHop ());// DMS : remove entries with the same destination (to unsure one route by destination) 
}

  std::pair<std::map<Ipv4Address, RoutingTableEntryUp>::iterator, bool> result = m_ipv4AddressEntry.insert (std::make_pair (rt.GetNextHop (), rt));//??? dst replaced by getway
  return result.second;
}


bool
RoutingTableUp::UpdateLifeTimeEntry (RoutingTableEntryUp & rt, Time lt)  //ADDED BY DMS
{
  NS_LOG_FUNCTION (this);
  std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
    m_ipv4AddressEntry.find (rt.GetDestination ());
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " fails; not found");
      return false;
    }
  i->second = rt;
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " lifetime update");
      i->second.SetLifeTime (lt);
  return true;
}


void
RoutingTableUp::DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface)
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4AddressEntry.empty ())
    {
      return;
    }
  for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
    {
      if (i->second.GetInterface () == iface)
        {
          std::map<Ipv4Address, RoutingTableEntryUp>::iterator tmp = i;
          ++i;
          m_ipv4AddressEntry.erase (tmp);
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableUp::Purge ()
{
  NS_LOG_FUNCTION (this);
uint16_t minHops=10000;
  if (m_ipv4AddressEntry.empty ())
    {
      return;
    }

          for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
                 m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
            {

              if (i->second.GetHop() < minHops)
                {
                      minHops= i->second.GetHop() ;
                }
               ++i;
            }

  for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
    {
      if ((i->second.GetLifeTime () < Seconds (0))/*||(i->second.GetHop ()>minHops)*/)
        {
              std::map<Ipv4Address, RoutingTableEntryUp>::iterator tmp = i;
              ++i;
              m_ipv4AddressEntry.erase (tmp);
       //       NS_LOG_UNCOND(" erase route: "<<Simulator::Now ()); //DMS2
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableUp::Purge (std::map<Ipv4Address, RoutingTableEntryUp> &table) const
{
  NS_LOG_FUNCTION (this);
  if (table.empty ())
    {
      return;
    }
  for (std::map<Ipv4Address, RoutingTableEntryUp>::iterator i =
         table.begin (); i != table.end (); )
    {
      if (i->second.GetLifeTime () < Seconds (0))
        {
              std::map<Ipv4Address, RoutingTableEntryUp>::iterator tmp = i;
              ++i;
              table.erase (tmp);
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableUp::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::map<Ipv4Address, RoutingTableEntryUp> table = m_ipv4AddressEntry;
  Purge (table);
  *stream->GetStream () << "\nDMSRP Routing table\n"
                        << "Destination\tGateway\t\tInterface\t\tExpire\t\tHops\tSeqNo\tCumEnergy\tMinSNR\n";
  for (std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i =
         table.begin (); i != table.end (); ++i)
    {
      i->second.Print (stream);
    }
  *stream->GetStream () << "\n";
}


void
RoutingTableUp::PrintInScreen ()  const //DMS added by dms
{
//NS_LOG_UNCOND("\nDMSRP Routing table\n" << "Destination\tGateway\t\tInterface\t\tExpire\t\tHops\tSeqNo\tCumEnergy\tMinSNR\n");

 std::map<Ipv4Address, RoutingTableEntryUp> table = m_ipv4AddressEntry;
  Purge (table);
  for (std::map<Ipv4Address, RoutingTableEntryUp>::const_iterator i =
         table.begin (); i != table.end (); ++i)
    {
      i->second.PrintInScreen ();
    }
 // *stream->GetStream () << "\n";
}


// end RoutingTableUp DMS ////////////////////////////////////////////////////




//   RoutingTable DMS ////////////////////////////////////////////////////////////////////////////////////////////////////

RoutingTableDown::RoutingTableDown (Time t)
{
}


bool
RoutingTableDown::IsEmpty ()     // DMS created by DMS
{
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      return true;
    }
 return false;

 
}


bool
RoutingTableDown::LookupRoute (Ipv4Address id, RoutingTableEntryDown & rt)
{
  NS_LOG_FUNCTION (this << id);
  Purge ();
  if (m_ipv4AddressEntry.empty ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found; m_ipv4AddressEntry is empty");
      return false;
    }
  std::map<Ipv4Address, RoutingTableEntryDown>::const_iterator i =
    m_ipv4AddressEntry.find (id);
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found");
      return false;
    }
  rt = i->second;
  NS_LOG_LOGIC ("Route to " << id << " found");
  return true;
}

bool
RoutingTableDown::DeleteRoute (Ipv4Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  Purge ();
  if (m_ipv4AddressEntry.erase (dst) != 0)
    {
      NS_LOG_LOGIC ("Route deletion to " << dst << " successful");
      return true;
    }
  NS_LOG_LOGIC ("Route deletion to " << dst << " not successful");
  return false;
}

bool
RoutingTableDown::AddRoute (RoutingTableEntryDown & rt)
{
  NS_LOG_FUNCTION (this);
  Purge ();
  m_ipv4AddressEntry.erase (rt.GetDestination ());// DMS : remove entries with the same destination (to unsure one route by destination)
  std::pair<std::map<Ipv4Address, RoutingTableEntryDown>::iterator, bool> result =
    m_ipv4AddressEntry.insert (std::make_pair (rt.GetDestination (), rt));//??? dst replaced by getway
  return result.second;
}

bool
RoutingTableDown::UpdateLifeTimeEntry (RoutingTableEntryDown & rt, Time lt)  //ADDED BY DMS
{
  NS_LOG_FUNCTION (this);
  std::map<Ipv4Address, RoutingTableEntryDown>::iterator i =
    m_ipv4AddressEntry.find (rt.GetDestination ());
  if (i == m_ipv4AddressEntry.end ())
    {
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " fails; not found");
      return false;
    }
  i->second = rt;
      NS_LOG_LOGIC ("Route update to " << rt.GetDestination () << " lifetime update");
      i->second.SetLifeTime (lt);
  return true;
}

void
RoutingTableDown::DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface)
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4AddressEntry.empty ())
    {
      return;
    }
  for (std::map<Ipv4Address, RoutingTableEntryDown>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
    {
      if (i->second.GetInterface () == iface)
        {
          std::map<Ipv4Address, RoutingTableEntryDown>::iterator tmp = i;
          ++i;
          m_ipv4AddressEntry.erase (tmp);
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableDown::Purge ()
{
  NS_LOG_FUNCTION (this);
  if (m_ipv4AddressEntry.empty ())
    {
      return;
    }
  for (std::map<Ipv4Address, RoutingTableEntryDown>::iterator i =
         m_ipv4AddressEntry.begin (); i != m_ipv4AddressEntry.end (); )
    {
      if (i->second.GetLifeTime () < Seconds (0))
        {

              std::map<Ipv4Address, RoutingTableEntryDown>::iterator tmp = i;
              ++i;
              m_ipv4AddressEntry.erase (tmp);
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableDown::Purge (std::map<Ipv4Address, RoutingTableEntryDown> &table) const
{
  NS_LOG_FUNCTION (this);
  if (table.empty ())
    {
      return;
    }
  for (std::map<Ipv4Address, RoutingTableEntryDown>::iterator i =
         table.begin (); i != table.end (); )
    {
      if (i->second.GetLifeTime () < Seconds (0))
        {

              std::map<Ipv4Address, RoutingTableEntryDown>::iterator tmp = i;
              ++i;
              table.erase (tmp);
        }
      else
        {
          ++i;
        }
    }
}

void
RoutingTableDown::Print (Ptr<OutputStreamWrapper> stream) const
{
  std::map<Ipv4Address, RoutingTableEntryDown> table = m_ipv4AddressEntry;
  Purge (table);
  *stream->GetStream () << "\nDMSRP Routing table\n"
                        << "Destination\tGateway\t\tInterface\tFlag\tExpire\t\tHops\n";
  for (std::map<Ipv4Address, RoutingTableEntryDown>::const_iterator i =
         table.begin (); i != table.end (); ++i)
    {
      i->second.Print (stream);
    }
  *stream->GetStream () << "\n";
}


void
RoutingTableDown::PrintInScreen ()  const //DMS added by dms
{
//NS_LOG_UNCOND("\nDMSRP Routing table\n" << "Destination\tGateway\t\tInterface\tExpire\t\tHops\nSeqNo\n");

 std::map<Ipv4Address, RoutingTableEntryDown> table = m_ipv4AddressEntry;
  Purge (table);
  for (std::map<Ipv4Address, RoutingTableEntryDown>::const_iterator i =
         table.begin (); i != table.end (); ++i)
    {
      i->second.PrintInScreen ();
    }
 // *stream->GetStream () << "\n";


}


// end RoutingTableDown DMS ////////////////////////////////////////////////////
}
}
