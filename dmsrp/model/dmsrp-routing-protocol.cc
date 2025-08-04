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
#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4) { std::clog << "[node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#include "ns3/core-module.h"
#include "ns3/basic-energy-source.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/energy-source-container.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/yans-wifi-helper.h"


#include "dmsrp-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/energy-module.h" //DMS
#include <algorithm>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DmsrpRoutingProtocol");

namespace dmsrp {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for DMSRP control traffic
const uint32_t RoutingProtocol::DMSRP_PORT = 651;



//-----------------------------------------------------------------------------
RoutingProtocol::RoutingProtocol ()
  : m_activeRouteTimeout (Seconds (6)),
    m_seqNo (0),
    m_hellotimer (Timer::CANCEL_ON_DESTROY),
    m_advertisetimer (Timer::CANCEL_ON_DESTROY),
  //  m_routingMode (MULTI_PARENT_MODE),     //DMS 
    m_routingTableUp (),     //DMS   //m_routingMode (3 modes):  BASIC_MODE , MULTI_PARENT_MODE , ENERGY_AWARE_MULTI_PARENT_MODE , SNR_AWARE_MULTI_PARENT_MODE
    m_routingTableDown (m_activeRouteTimeout),     //DMS
    m_maxHelloHops(1000),  // DMS added by DMS
    m_maxAdvertiseHops(1000),  // DMS added by DMS
    m_helloTimerExpire(Seconds (5)), //DMS 
    m_advertiseTimerExpire(Seconds (4)) //DMS

{
  
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::dmsrp::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Dmsrp")
    .AddConstructor<RoutingProtocol> ()
    .AddAttribute ("ActiveRouteTimeout", "Period of time during which the route is considered to be valid",
                   TimeValue (Seconds (6)),
                   MakeTimeAccessor (&RoutingProtocol::m_activeRouteTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("EnableBroadcast", "Indicates whether a broadcast data packets forwarding enable.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&RoutingProtocol::SetBroadcastEnable,
                                        &RoutingProtocol::GetBroadcastEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("UniformRv",
                   "Access to the underlying UniformRandomVariable",
                   StringValue ("ns3::UniformRandomVariable"),
                   MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
                   MakePointerChecker<UniformRandomVariable> ())
    .AddAttribute ("IsSink", "Indicates if this node will is a server or not.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RoutingProtocol::SetIsSink,
                                        &RoutingProtocol::GetIsSink),
                   MakeBooleanChecker ())
    .AddAttribute ("IsServer", "Indicates if this node will is a server or not.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&RoutingProtocol::SetIsServer,
                                        &RoutingProtocol::GetIsServer),
                   MakeBooleanChecker ())
    .AddAttribute ("ServerAddress", "Server ADDRESS",
                   Ipv4AddressValue ("10.1.0.20"),
                   MakeIpv4AddressAccessor (&RoutingProtocol::SetServerAddress,
                                        &RoutingProtocol::GetServerAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("SinkGateWayAddress", "Sink gateway address",
                   Ipv4AddressValue ("10.1.0.20"),
                   MakeIpv4AddressAccessor (&RoutingProtocol::SetSinkGateWayAddress,
                                        &RoutingProtocol::GetSinkGateWayAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("SinkOutPutDevice",
                   "Sink OutPut Device",
                   StringValue ("ns3::NetDevice"),
                   MakePointerAccessor (&RoutingProtocol::m_sinkOutPutDevice),
                   MakePointerChecker<NetDevice> ())
    .AddAttribute ("SnrWeight", "Snr wheight (importance)",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RoutingProtocol::m_snrWeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("EnergyWeight", "Energy wheight (importance)",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RoutingProtocol::m_snrWeight),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Routingmode", "Routing mode",
                   UintegerValue (SNR_AWARE_MULTI_PARENT_MODE),
                   MakeUintegerAccessor (&RoutingProtocol::m_routingMode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RngRep", "Routing rng seed (repetition)",
                   UintegerValue (SNR_AWARE_MULTI_PARENT_MODE),
                   MakeUintegerAccessor (&RoutingProtocol::m_rngRep),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("DepTime", "Depletion Time",
                   MakeTraceSourceAccessor (&RoutingProtocol::m_depTimeTrace),
                   "ns3::dmsrp::RoutingProtocol::TableChangeTracedCallback")

  ; 
  return tid;
}



RoutingProtocol::~RoutingProtocol ()
{
}

void
RoutingProtocol::DoDispose ()
{
  m_ipv4 = 0;
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter =
         m_socketAddresses.begin (); iter != m_socketAddresses.end (); iter++)
    {
      iter->first->Close ();
    }
  m_socketAddresses.clear ();
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter =
         m_socketSubnetBroadcastAddresses.begin (); iter != m_socketSubnetBroadcastAddresses.end (); iter++)
    {
      iter->first->Close ();
    }
  m_socketSubnetBroadcastAddresses.clear ();
  Ipv4RoutingProtocol::DoDispose ();
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
                        << "; Time: " << Now ().As (unit)
                        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
                        << ", DMSRP Routing table" << std::endl;

  m_routingTableDown.Print (stream);
  *stream->GetStream () << std::endl;
}

int64_t
RoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);


}

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{

  NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
  if (!p)
    {
      NS_LOG_DEBUG ("Packet is == 0");
      return LoopbackRoute (header, oif); // later
    }
  if (m_socketAddresses.empty ())
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
      NS_LOG_LOGIC ("No dmsrp interfaces");
      Ptr<Ipv4Route> route;
      return route;
    }
  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route;
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();
origin = header.GetSource ();
  RoutingTableEntryDown rt;
  RoutingTableEntryUp rtUp;


                 if (m_routingTableDown.LookupRoute (dst, rt))
                    {

                      
                      route = rt.GetRoute ();

                     NS_ASSERT (route != 0);
                      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());

                      if (oif != 0 && route->GetOutputDevice () != oif)
                        {
                          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
                          NS_LOG_UNCOND ("Output device doesn't match. Dropped.");
                          sockerr = Socket::ERROR_NOROUTETOHOST;
                          return Ptr<Ipv4Route> ();
                        }

         //    NS_LOG_UNCOND ("OUT packet : origin "<<origin << " dst " << dst <<"  "<<Simulator::Now ());  ////DMS2 
                      return route;
                    }

             if (!m_isServer)
              {
                 if (!m_routingTableUp.IsEmpty())
                    {

                      m_routingTableUp.LookupBestRoute(rtUp);
                      route = rtUp.GetRoute ();

                     NS_ASSERT (route != 0);
                      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());

                      if (oif != 0 && route->GetOutputDevice () != oif)
                        {
                          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
                          NS_LOG_UNCOND ("Output device doesn't match. Dropped.");
                          sockerr = Socket::ERROR_NOROUTETOHOST;
                          return Ptr<Ipv4Route> ();
                        }
                      return route;
                    }
              }


                      return route;
}



bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
{



  NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No dmsrp interfaces");
      return false;
    }
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP


  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);


        Ipv4Header header2;
        Ptr<Packet> p2 = p->Copy();
        //NS_LOG_UNCOND ("Packet size: "<<p->GetSize());


        //NS_LOG_UNCOND ( "server:" << m_serverAdress);
/*       if(m_isServer)
        {

          NS_LOG_UNCOND ("Packet size1: "<<p2->GetSize());

          p2->PeekHeader (header2);
          NS_LOG_UNCOND ("Packet size2: "<<p2->GetSize());
      NS_LOG_UNCOND ("yes h0: "<<" src: "<< header.GetSource ()  <<" dst: "<<header.GetDestination ());
      NS_LOG_UNCOND ("yes h1: "<<" src: "<< header2.GetSource ()  <<" dst: "<<header2.GetDestination ());

        }*/

       if(m_isServer|| (m_isSink && m_sinkOutPutDevice==idev))
        {

          //NS_LOG_UNCOND ("Packet size1: "<<p2->GetSize());

          p2->RemoveHeader (header2);
          //NS_LOG_UNCOND ("Packet size2: "<<p2->GetSize());
      //NS_LOG_UNCOND ("yes h0: "<<" src: "<< header.GetSource ()  <<" dst: "<<header.GetDestination ());
      //NS_LOG_UNCOND ("yes h1: "<<" src: "<< header2.GetSource ()  <<" dst: "<<header2.GetDestination ());
   //       p2->RemoveHeader (header2);
    //      NS_LOG_UNCOND ("Packet size3: "<<p2->GetSize());
   //   NS_LOG_UNCOND ("yes h2: "<<" src: "<< header2.GetSource ()  <<" dst: "<<header2.GetDestination ());
        }
        else
        {
          header2=header;
        }



  Ipv4Address dst = header2.GetDestination ();
  Ipv4Address origin = header2.GetSource ();

/*
  if (IsMyOwnAddress (header.GetDestination ()))
    {
       NS_LOG_UNCOND ("MYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY ADDRESS");
     // return true;
    }
*/

  // Duplicate of own packet
  if (IsMyOwnAddress (origin))
    {
  //      NS_LOG_UNCOND ("cooooooode0.5");
      return true;
    }

  // DMSRP is not a multicast routing protocol
  if (dst.IsMulticast ())
    {
  //      NS_LOG_UNCOND ("cooooooode0.6");
      return false;
    }



  // Broadcast local delivery/forwarding
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()) == iif)
        {
          if (dst == iface.GetBroadcast () || dst.IsBroadcast ())
            {

/*
              if (m_dpd.IsDuplicate (p, header))
                {
                  NS_LOG_UNCOND ("cooooooode1.1");
                  NS_LOG_DEBUG ("Duplicated packet " << p->GetUid () << " from " << origin << ". Drop.");
                  NS_LOG_UNCOND ("Duplicated packet " << p->GetUid () << " from " << origin << ". Drop.");
                  return true;
                }
*/
              Ptr<Packet> packet =  p2->Copy ();
              if (lcb.IsNull () == false)
                {
                  //NS_LOG_UNCOND ("cooooooode2");
                  NS_LOG_LOGIC ("Broadcast local delivery to " << iface.GetLocal ());
                  //NS_LOG_UNCOND ("Broadcast local delivery to " << iface.GetLocal ());
                  lcb ( p2, header2, iif);
                  // Fall through to additional processing
                }
              else
                {
                  //NS_LOG_UNCOND ("cooooooode3");
                  NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " <<  p2->GetUid () << " from " << origin);
                  //NS_LOG_UNCOND ("Unable to deliver packet locally due to null callback " <<  p2->GetUid () << " from " << origin);
                  ecb ( p2, header2, Socket::ERROR_NOROUTETOHOST);
                }
              if (!m_enableBroadcast)
                {
                    //NS_LOG_UNCOND ("cooooooode3.2");
                  return true;
                }
              if (header2.GetProtocol () == UdpL4Protocol::PROT_NUMBER)
                {
                  UdpHeader udpHeader;
                   p2->PeekHeader (udpHeader);
                  if (udpHeader.GetDestinationPort () == DMSRP_PORT)
                    {
                      // DMSRP packets sent in broadcast are already managed
                      //NS_LOG_UNCOND ("cooooooode3.3");
                      return true;
                    }
                }
              if (header2.GetTtl () > 1)
                {
                  NS_LOG_LOGIC ("Forward broadcast. TTL " << (uint16_t) header2.GetTtl ());
                  //NS_LOG_UNCOND ("Forward broadcast. TTL " << (uint16_t) header2.GetTtl ());
                  RoutingTableEntryDown toBroadcast;
                  if (m_routingTableDown.LookupRoute (dst, toBroadcast))
                    {
                      //NS_LOG_UNCOND ("cooooooode4");
                      Ptr<Ipv4Route> route = toBroadcast.GetRoute ();
                      ucb (route, packet, header2);
                    }
                  else
                    {
                      //NS_LOG_UNCOND ("cooooooode4.2");
                      NS_LOG_DEBUG ("No route to forward broadcast. Drop packet " <<  p2->GetUid ());
                      //NS_LOG_UNCOND ("No route to forward broadcast. Drop packet " <<  p2->GetUid ());
                    }
                }
              else
                {
                  NS_LOG_DEBUG ("TTL exceeded. Drop packet " <<  p2->GetUid ());
                }
               //NS_LOG_UNCOND ("cooooooode4.3");  
              return true;
            }
        }
    }

  // Unicast local delivery


  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      
      if (lcb.IsNull () == false)
        {
          //NS_LOG_UNCOND ("cooooooode5");
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          //NS_LOG_UNCOND ("Unicast local delivery to " << dst);
          lcb ( p2, header2, iif);
        }
      else
        {
          //NS_LOG_UNCOND ("cooooooode6");
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " <<  p2->GetUid () << " from " << origin);
          //NS_LOG_UNCOND ("Unable to deliver packet locally due to null callback " <<  p2->GetUid () << " from " << origin);
          ecb ( p2, header2, Socket::ERROR_NOROUTETOHOST);
        }
      return true;
    }


  // Check if input device supports IP forwarding
  if (m_ipv4->IsForwarding (iif) == false)
    {
      //NS_LOG_UNCOND ("cooooooode7");
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb ( p2, header2, Socket::ERROR_NOROUTETOHOST);
      return true;
    }

  // Forwarding
// NS_LOG_UNCOND ("FORWARDIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIG "<< idev->GetAddress () <<" TO ");
 //NS_LOG_UNCOND ("data type:"<< int(header.GetProtocol())<< "   IN: "<<idev->GetAddress () <<"       src: "<<header.GetSource ()<< "--> dst: "<<header.GetDestination ()<< "    "<<Simulator::Now ());


        if(m_isSink && m_sinkOutPutDevice==idev)
        {
          return Forwarding ( p2, header, ucb, ecb, true); 
        }
        else
        {
          return Forwarding ( p2, header, ucb, ecb, false); 
        }


}

bool
RoutingProtocol::Forwarding (Ptr<const Packet> p, const Ipv4Header & header,
                             UnicastForwardCallback ucb, ErrorCallback ecb,bool PcktFromServer)
{
  NS_LOG_FUNCTION (this);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();
  m_routingTableUp.Purge ();
  m_routingTableDown.Purge ();
  RoutingTableEntryDown toDst;
  RoutingTableEntryUp toDstUp;
  Ptr<Ipv4Route> route=Create<Ipv4Route>();

if (!m_isServer)
{    

    if (!m_isSink)
    {
        if(m_routingTableDown.LookupRoute (dst, toDst))
        {
          route = toDst.GetRoute ();
          NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

          RoutingTableEntryDown toOrigin;
          m_routingTableDown.LookupRoute (origin, toOrigin);
          ucb (route, p, header);
          return true;
        
        }
        else
        {
            if(!m_routingTableUp.IsEmpty())
            {

                        m_routingTableUp.LookupBestRoute(toDstUp);
                        route = toDstUp.GetRoute ();

                  NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

                  ucb (route, p, header)       ;
                  return true;
            }
            else
            {
      //          NS_LOG_UNCOND ("THERE IS NO route to the sink" );
                return false; // 
            }       
        }
    }
    else
    {

         if(m_routingTableDown.LookupRoute (dst, toDst))
         {
          route = toDst.GetRoute ();
          NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

          RoutingTableEntryDown toOrigin;
          m_routingTableDown.LookupRoute (origin, toOrigin);

          ucb (route, p, header);
          return true;
         }
         else    // should be sent to the server
         {
             if(!PcktFromServer)
             {
                //    NS_LOG_UNCOND ("SHOULD BE SENT TO THE STATION");
                    
               if(m_routingTableUp.LookupBestRoute(toDstUp))
               {                    
                   
                   route = toDstUp.GetRoute ();

////////////////////
                                Ptr<Packet> p2 = p->Copy();
                                Ipv4Header newIpv4Header =Ipv4Header();
                                newIpv4Header.SetSource (Ipv4Address("41.110.3.26"));
                                 newIpv4Header.SetProtocol (4);
                                 newIpv4Header.SetPayloadSize (header.GetPayloadSize()+20);
                                newIpv4Header.SetDestination (/*header.GetDestination()*/Ipv4Address("41.110.5.89") /*dst Ipv4Address("10.1.0.109")*/);  //
                                p2->AddHeader (header);
///////////////////////
                                //NS_LOG_UNCOND ("prot: "<<(int)header.GetProtocol()<<"Taille: "<< p->GetSize()<<"nouvelle taille: "<<p2->GetSize());

                                  NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

                                  ucb (route, p2, /*header*/newIpv4Header)       ;
                    //                 NS_LOG_UNCOND ("IT HAS BEEN SENT TO THE STATION");
                                  return true;

                }
                else
                {
    //                    NS_LOG_UNCOND ("no route to the server");
                        return false;
                }
              }
              else
              {
                        //NS_LOG_UNCOND ("data pckt came from the server");
                        return false;
              }

         }
        }
}
else //if it is the server
{


/////////////////////////
                             //   Ptr<Packet> p2 = p->Copy();
                             //   Ipv4Header oldIpv4Header =Ipv4Header();
                             //   p->PeekHeader (oldIpv4Header);            // to extract the orinal dst adr
    //   if( (m_isSink && m_sinkOutPutDevice==idev))
   //       p2->RemoveHeader (header2);
    //      NS_LOG_UNCOND ("Packet size3: "<<p2->GetSize());
   //   NS_LOG_UNCOND ("yes h2: "<<" src: "<< header2.GetSource ()  <<" dst: "<<header2.GetDestination ());
        
/////////////////////////


        if(m_routingTableDown.LookupRoute (header.GetDestination(), toDst))
        {


          route = toDst.GetRoute ();
          //NS_LOG_UNCOND ("route gateway "<< route->GetGateway() );
//////////////////////
                                 Ptr<Packet> p2 = p->Copy();
                                Ipv4Header newIpv4Header =Ipv4Header();
                           //     p2->RemoveHeader (oldIpv4Header);
                              //  newIpv4Header=header;
                                newIpv4Header.SetSource (origin);
                                 newIpv4Header.SetProtocol (4);
                                 newIpv4Header.SetPayloadSize (header.GetPayloadSize()+20);
                                newIpv4Header.SetDestination (/*m_serverAdress dst  route->GetGateway() Ipv4Address("10.1.0.114")*/header.GetDestination());

                       //          p2->RemoveHeader(header);                            
                                p2->AddHeader (header);
//route->SetSource ( Ipv4Address("10.1.0.106"));
//route->SetDestination (route->GetGateway());
///////////////////////
          NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());
          RoutingTableEntryDown toOrigin;
          m_routingTableDown.LookupRoute (origin, toOrigin);
          ucb (route, p2, newIpv4Header);
          return true;
        
        }
        else
        {
    //                     NS_LOG_UNCOND ("no route from the server to the destination");               
        }


}
      
  NS_LOG_LOGIC ("route not found to " << dst << ". Send RERR message.");
  NS_LOG_DEBUG ("Drop packet " << p->GetUid () << " because no route to forward it.");
  return false;
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  // Remember lo route


              RoutingTableEntryDown newEntry (/*device=*/ m_lo, /*dst=*/Ipv4Address::GetLoopback (),/*seqNo=*/ 0,
                                                      /*iface=*/ Ipv4InterfaceAddress (Ipv4Address::GetLoopback (), Ipv4Mask ("255.0.0.0")), /*hops=*/ 1,
                                                      /*nextHop*/ Ipv4Address::GetLoopback (), /*timeLife=*/ Simulator::GetMaximumSimulationTime ());
              m_routingTableDown.AddRoute (newEntry);


  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (i) > 1)
    {
      NS_LOG_WARN ("DMSRP does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                             UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp, this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetLocal (), DMSRP_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketAddresses.insert (std::make_pair (socket, iface));

  // create also a subnet broadcast socket
  socket = Socket::CreateSocket (GetObject<Node> (),
                                 UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp, this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetBroadcast (), DMSRP_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

  // Add local broadcast record to the routing table
  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

              RoutingTableEntryDown newEntry (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (),/*seqNo=*/ 0,
                                                      /*iface=*/ iface, /*hops=*/ 1,
                                                      /*nextHop*/ iface.GetBroadcast (), /*timeLife=*/ Simulator::GetMaximumSimulationTime ());
              m_routingTableDown.AddRoute (newEntry);



  // Allow neighbor manager use this interface for layer 2 feedback if possible
  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
  if (wifi == 0)
    {
      return;
    }
  Ptr<WifiMac> mac = wifi->GetMac ();
  if (mac == 0)
    {
      return;
    }


}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());



  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketAddresses.erase (socket);

  // Close socket
  socket = FindSubnetBroadcastSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketSubnetBroadcastAddresses.erase (socket);

  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No dmsrp interfaces");
      m_hellotimer.Cancel ();
      m_routingTableUp.Clear ();
      m_routingTableDown.Clear ();
      return;
    }
  m_routingTableDown.DeleteAllRoutesFromInterface (m_ipv4->GetAddress (i, 0));
  m_routingTableUp.DeleteAllRoutesFromInterface (m_ipv4->GetAddress (i, 0));
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << " interface " << i << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (i))
    {
      return;
    }
  if (l3->GetNAddresses (i) == 1)
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
      Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
      if (!socket)
        {
          if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
            {
              return;
            }
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp,this));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), DMSRP_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          // create also a subnet directed broadcast socket
          socket = Socket::CreateSocket (GetObject<Node> (),
                                         UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp, this));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetBroadcast (), DMSRP_PORT));
          socket->SetAllowBroadcast (true);
          socket->SetIpRecvTtl (true);
          m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

          // Add local broadcast record to the routing table
          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (
              m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

              RoutingTableEntryDown newEntry (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (),/*seqNo=*/ 0,
                                                      /*iface=*/ iface, /*hops=*/ 1,
                                                      /*nextHop*/ iface.GetBroadcast (), /*timeLife=*/ Simulator::GetMaximumSimulationTime ());
              m_routingTableDown.AddRoute (newEntry);


        }
    }
  else
    {
      NS_LOG_LOGIC ("DMSRP does not work with more then one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
  if (socket)
    {
      m_routingTableDown.DeleteAllRoutesFromInterface (address);
      socket->Close ();
      m_socketAddresses.erase (socket);

      Ptr<Socket> unicastSocket = FindSubnetBroadcastSocketWithInterfaceAddress (address);
      if (unicastSocket)
        {
          unicastSocket->Close ();
          m_socketAddresses.erase (unicastSocket);
        }

      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
      if (l3->GetNAddresses (i))
        {
          Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp, this));
          // Bind to any IP address so that broadcasts can be received
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), DMSRP_PORT));
          socket->SetAllowBroadcast (true);
          socket->SetIpRecvTtl (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));

          // create also a unicast socket
          socket = Socket::CreateSocket (GetObject<Node> (),
                                         UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvDmsrp, this));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetBroadcast (), DMSRP_PORT));
          socket->SetAllowBroadcast (true);
          socket->SetIpRecvTtl (true);
          m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

          // Add local broadcast record to the routing table
          Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

              RoutingTableEntryDown newEntry (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (),/*seqNo=*/ 0,
                                                      /*iface=*/ iface, /*hops=*/ 1,
                                                      /*nextHop*/ iface.GetBroadcast (), /*timeLife=*/ Simulator::GetMaximumSimulationTime ());
              m_routingTableDown.AddRoute (newEntry);
        }
      if (m_socketAddresses.empty ())
        {
          NS_LOG_LOGIC ("No dmsrp interfaces");

          m_hellotimer.Cancel ();
          m_advertisetimer.Cancel ();

          m_routingTableUp.Clear ();
          m_routingTableDown.Clear ();
          return;
        }
    }
  else
    {
      NS_LOG_LOGIC ("Remove address not participating in DMSRP operation");
    }
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{

 // NS_LOG_UNCOND ("looped back");
  NS_LOG_FUNCTION (this << hdr);
  NS_ASSERT (m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());
  //
  // Source address selection here is tricky.  The loopback route is
  // returned when DMSRP does not have a route; this causes the packet
  // to be looped back and handled (cached) in RouteInput() method
  // while a route is found. However, connection-oriented protocols
  // like TCP need to create an endpoint four-tuple (src, src port,
  // dst, dst port) and create a pseudo-header for checksumming.  So,
  // DMSRP needs to guess correctly what the eventual source address
  // will be.
  //
  // For single interface, single address nodes, this is not a problem.
  // When there are possibly multiple outgoing interfaces, the policy
  // implemented here is to pick the first available DMSRP interface.
  // If RouteOutput() caller specified an outgoing interface, that
  // further constrains the selection of source address
  //
  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
          if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
            {
              rt->SetSource (addr);
              break;
            }
        }
    }
  else
    {
      rt->SetSource (j->second.GetLocal ());
    }
  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid DMSRP source address not found");
  rt->SetGateway (Ipv4Address ("127.0.0.1"));
  rt->SetOutputDevice (m_lo);
  return rt;
}

void
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  socket->SendTo (packet, 0, InetSocketAddress (destination, DMSRP_PORT));

}


void
RoutingProtocol::RecvDmsrp (Ptr<Socket> socket)
{

/*
if(m_routingMode==BASIC_MODE)
{
NS_LOG_UNCOND("Mode is: BASIC_MODE" <<BASIC_MODE);
}

if(m_routingMode==MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mode is: MULTI_PARENT_MODE" <<MULTI_PARENT_MODE);
}

if(m_routingMode==ENERGY_AWARE_MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mode is: ENERGY_AWARE_MULTI_PARENT_MODE" <<ENERGY_AWARE_MULTI_PARENT_MODE);
}

if(m_routingMode==SNR_AWARE_MULTI_PARENT_MODE)
{
NS_LOG_UNCOND("Mode is: SNR_AWARE_MULTI_PARENT_MODE" <<SNR_AWARE_MULTI_PARENT_MODE);
}*/

/*if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.16"))
{
 NS_LOG_UNCOND("NODE 16 received iot packet: "<<Simulator::Now ());

} */
  NS_LOG_FUNCTION (this << socket);
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  Ipv4Address receiver;

  if (m_socketAddresses.find (socket) != m_socketAddresses.end ())
    {
      receiver = m_socketAddresses[socket].GetLocal ();
    }
  else if (m_socketSubnetBroadcastAddresses.find (socket) != m_socketSubnetBroadcastAddresses.end ())
    {
      receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal ();
     // NS_LOG_UNCOND ("BRODCAST");
    }
  else
    {
      NS_ASSERT_MSG (false, "Received a packet from an unknown socket");
    }
  NS_LOG_DEBUG ("DMSRP node " << this << " received a DMSRP packet from " << sender << " to " << receiver);
 // NS_LOG_UNCOND ("DMSRP node " << this << " received a DMSRP packet from " << sender << " to " << receiver);

  TypeHeader tHeader (DMSRPTYPE_HELLO);
  packet->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
 //    NS_LOG_UNCOND("header invalid" );
      NS_LOG_DEBUG ("DMSRP message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return; // drop
    }
  switch (tHeader.Get ())
    {
    case DMSRPTYPE_HELLO://DMS
      {
       RecvHello (packet, receiver, sender);// DMS
        break;
      }
    case DMSRPTYPE_ADVERTISE://DMS
      {
       RecvAdvertise (packet, receiver, sender);// DMS
        break;
      }
    case DMSRPTYPE_SRVADVERTISE://DMS
      {
       RecvSrvAdvertise (packet, receiver, sender);// DMS
        break;
      }
    }
}



void //DMS
RoutingProtocol::RecvHello (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src)
{
      //  NS_LOG_UNCOND( Simulator::Now () <<":"<<(m_socketAddresses.begin()->second).GetLocal () <<" HELLO received  DMS --- source:" << src << " receiver:"  << receiver );

 NS_LOG_FUNCTION (this);
  HelloHeader helloHeader;
  p->RemoveHeader (helloHeader);
// uint32_t id = helloHeader.GetId ();  //DMS
  Ipv4Address origin = helloHeader.GetOrigin ();

  m_routingTableUp.Purge ();
 
 // RoutingTableEntryDown toOrigin;
 

       RoutingTableEntryUp myRoutingEntry;


      // ***** SNR **** // 
      SnrTag tag;
      if (p->PeekPacketTag(tag))
      {
        temp_snr=tag.Get();    
    // NS_LOG_UNCOND("Reveived hello packet-> MinSnr: "<<helloHeader.GetMinSnr()<<"  pckt Snr: "<<tag.Get()<< "  -  Received Cum Energy: "<<helloHeader.GetMinEnergy ()<< "  -Stored cum Energy: "<<m_minEnergy<< "  -Remaining Energy: "<<EnergySrc->GetRemainingEnergy ());     
      }  
if(temp_snr<helloHeader.GetMinSnr ())
{
helloHeader.SetMinSnr (temp_snr);
}
      // ***** SNR **** //


    //   m_routingTableUp.LookupTheRoute(myRoutingEntry);

   //     
// NS_LOG_UNCOND(receiver<<"A NODE received hello: "<<Simulator::Now ());
/*if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.4"))
{
 NS_LOG_UNCOND("NODE 4 received hello: "<<Simulator::Now ());
m_routingTableUp.PrintInScreen();

} */

 /*
   *  Node checks to determine whether it has received a HELLO with the same Originator IP Address and HELLO ID.
   *  If such a HELLO has been received, the node silently discards the newly received HELLO.
   */
  if (m_isSink||m_isServer)
  {
     NS_LOG_DEBUG (receiver<<" Ignoring HELLO because this node has an output access or is the server");
 //    NS_LOG_UNCOND(receiver<<" Ignoring HELLO because this node has an output access or is the server");
     return;
  }
/*  if (m_helloIdCache.IsDuplicate (origin, id))
  {
     NS_LOG_DEBUG (receiver<<" Ignoring HELLO due to duplicate");
  //   NS_LOG_UNCOND(receiver<< " Ignoring HELLO due to duplicate ----------------------------");
     return;
  }*/

  // Increment RREQ hop count
  uint8_t hop = helloHeader.GetHopCount () + 1; //ForwardHello2 helloHeader
  helloHeader.SetHopCount (hop);

  if (hop>m_maxHelloHops)
  {
     NS_LOG_DEBUG (receiver<<" Ignoring HELLO: hops> maxhops");
  //   NS_LOG_UNCOND(receiver<<" Ignoring HELLO: hops> maxhops");
     return;
  }
   
      Ptr<EnergySource> EnergySrc;
      EnergySrc = this->GetObject<EnergySourceContainer> ()->Get (0);

/*if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.33"))
{
 // NS_LOG_UNCOND (receiver<<"its minEnergiy is: "<<  m_minEnergy<<"----Reveive HELLO packetfrom:"<<src<< " remaining enrgy: "<<EnergySrc->GetRemainingEnergy ()<<" rem energy: "<<helloHeader.GetMinEnergy()<<"  minSNR: "<<helloHeader.GetMinSnr ()<<" and hop="<<(int)hop); //DMS2
// NS_LOG_UNCOND("----ROUTING TABLE2: "<<(m_socketAddresses.begin()->second).GetLocal () <<"   "<<Simulator::Now ());
//m_routingTableUp.PrintInScreen(); //DMS2
}  
  
if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.4"))
{
 // NS_LOG_UNCOND (receiver<<"  : "<< " remaining enrgy: "<<EnergySrc->GetRemainingEnergy ());

} */



bool modifyRoutingTable=false;
//ool forwardHelloHeader=false;

if((m_routingTableUp.IsEmpty()))
{
modifyRoutingTable=true;
//forwardHelloHeader=true;
 m_Snr=0;
m_minEnergy=9999999.9;
}
else
{       
        m_routingTableUp.LookupBestRoute(myRoutingEntry);

        if((myRoutingEntry.GetDestination()==origin) )
        {
                if((myRoutingEntry.GetSeqNo()>helloHeader.GetOriginSeqno()))
                {
                         return;        
                }         
        }

  /*      if(myRoutingEntry.GetNextHop()==src)
        {
              modifyRoutingTable=true;
 //             forwardHelloHeader=true;
        }
*/



        if((m_routingTableUp.GetMinHops()>=helloHeader.GetHopCount())) 
        {
        modifyRoutingTable=true;
        }

/*        if((myRoutingEntry.GetHop()>=helloHeader.GetHopCount())) 
        {
        modifyRoutingTable=true;
        }*/
          
}

    
if(modifyRoutingTable)

{
       /* if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.3"))
        {
   //       NS_LOG_UNCOND (receiver<<"----Route pdated for this node:");
   //      NS_LOG_UNCOND("----uproute is to: "<<src);
        }  */
 
      Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));

      RoutingTableEntryUp newEntry (/*device=*/ dev, /*dst=*/ origin,/*seqNo=*/ helloHeader.GetOriginSeqno (),
                                              /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0), /*hops=*/ hop,
                                              /*nextHop*/ src, /*timeLife=*/ m_activeRouteTimeout,helloHeader.GetMinEnergy () ,std::min (helloHeader.GetMinSnr (),(float) temp_snr ) );
      m_routingTableUp.AddRoute (newEntry);

         helloHeader.SetMinEnergy (helloHeader.GetMinEnergy ()+EnergySrc->GetRemainingEnergy ());
         m_minEnergy=helloHeader.GetMinEnergy ();
         m_Snr=helloHeader.GetMinSnr ();
  Ptr<OutputStreamWrapper> testprint = Create<OutputStreamWrapper>("routingtestprint", std::ios::out);

  }

}


void //DMS
RoutingProtocol::RecvAdvertise (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src)  // this function is used by simple node and sinks
{

 SrvAdvertiseHeader srvadvertiseHeader;
  Ptr<Ipv4Route> route=Create<Ipv4Route>();
    RoutingTableEntryUp toDst;
 


  if(!m_isServer)
  {


          // NS_LOG_UNCOND( Simulator::Now () <<":"<<(m_socketAddresses.begin()->second).GetLocal () <<" Advertise received  DMS --- source:" << src << " receiver:"  << receiver );

         NS_LOG_FUNCTION (this);
          AdvertiseHeader advertiseHeader;
          p->RemoveHeader (advertiseHeader);
   //       uint32_t id = advertiseHeader.GetId ();  //DMS
          Ipv4Address origin = advertiseHeader.GetOrigin ();

          // Increment RREQ hop count
          uint8_t hop = advertiseHeader.GetHopCount () + 1; //ForwardHello2 advertiseHeader
          advertiseHeader.SetHopCount (hop);

          if (hop>m_maxAdvertiseHops)
          {
             NS_LOG_DEBUG (receiver<<" Ignoring ADVERTISE: hops> maxhops");
      //       NS_LOG_UNCOND(receiver<<" Ignoring ADVERTISE: hops> maxhops");
             return;
          }



               RoutingTableEntryDown myRoutingEntry;

              Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));




                bool modifyRoutingTable=false;

              

                if(m_routingTableDown.LookupRoute (origin, myRoutingEntry))
                {
                        if(((myRoutingEntry.GetDestination()==origin) &&(myRoutingEntry.GetSeqNo()<advertiseHeader.GetOriginSeqno())))
                        {
                               modifyRoutingTable=true;           
                        }
                        else
                        {
                             return;
                        }
                        if((myRoutingEntry.GetHop()>advertiseHeader.GetHopCount())) 
                        {
                        modifyRoutingTable=true;
                        }

                        if(myRoutingEntry.GetLifeTime ()<=Seconds(0))
                        {
                               modifyRoutingTable=true;   
        //                       NS_LOG_UNCOND(receiver<< " RT3 ROUTE EXPIRE");        
                        }              

                }
                else
                {
                        modifyRoutingTable=true;                       
                }


       if(modifyRoutingTable)  /// modify the route
        {

              RoutingTableEntryDown newEntry (/*device=*/ dev, /*dst=*/ origin,/*seqNo=*/ advertiseHeader.GetOriginSeqno (),
                                                      /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0), /*hops=*/ hop,
                                                      /*nextHop*/ src, /*timeLife=*/ m_activeRouteTimeout);
              m_routingTableDown.AddRoute (newEntry);

        }
         
   //      m_routingTableDown.PrintInScreen();
         
        if(!m_isSink)
        {
          ForwardAdvertise (advertiseHeader);  //forward to another node or to the sink
        }
        else   // send to the server by adding to the header the
        {

        //  NS_LOG_UNCOND ("A SINK: " << receiver  <<" HAS RECEIVED AN ADVERTISE"<<" src-->dst: "<<src<< "-->"<<receiver<< " pcktID: "<<p->GetUid ()<< "    "<<Simulator::Now ());
             if((!m_routingTableUp.IsEmpty())) 
             {       

     
                              m_routingTableUp.LookupBestRoute(toDst);
                              route = toDst.GetRoute ();
                        
                              SrvAdvertiseHeader srvadvertiseHeader ( /*hops=*/ advertiseHeader.GetHopCount(), /*prefix size=*/ 0, /*orig=*/ advertiseHeader.GetOrigin (), /*sink=*/ route->GetSource(),/*seqno=*/ advertiseHeader.GetSeqno());
                              Ptr<Packet> packet = Create<Packet> ();
                              SocketIpTtlTag tag;
                              tag.SetTtl (1);
                              packet->AddPacketTag (tag);
                              packet->AddHeader (srvadvertiseHeader);
                              TypeHeader tHeader (DMSRPTYPE_SRVADVERTISE);
                              packet->AddHeader (tHeader);

                              ForwardSrvAdvertise (srvadvertiseHeader);                          

             }
        }

        return;
   
  }


}



void //DMS
RoutingProtocol::RecvSrvAdvertise (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src)
{
  
  if(m_isServer)
  {
 //m_routingTableDown.PrintInScreen();

    

   // NS_LOG_UNCOND( Simulator::Now () <<":"<<(m_socketAddresses.begin()->second).GetLocal () <<" SRVAdvertise received  DMS --- source:" << src << " receiver:"  << receiver );

         NS_LOG_FUNCTION (this);
          SrvAdvertiseHeader srvadvertiseHeader;
          p->RemoveHeader (srvadvertiseHeader);
  //        uint32_t id = srvadvertiseHeader.GetId ();  //DMS
          Ipv4Address origin = srvadvertiseHeader.GetOrigin ();
          Ipv4Address sink = srvadvertiseHeader.GetSink ();

          // Increment RREQ hop count
          uint8_t hop = srvadvertiseHeader.GetHopCount () + 1; //ForwardHello2 advertiseHeader
          srvadvertiseHeader.SetHopCount (hop);


               RoutingTableEntryDown myRoutingEntry;

              Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));



               bool modifyRoutingTable=false;

              

                if(m_routingTableDown.LookupRoute (origin, myRoutingEntry))
                {
               
                        if(((myRoutingEntry.GetDestination()==origin) &&(myRoutingEntry.GetSeqNo()<srvadvertiseHeader.GetOriginSeqno())))
                        {
                               modifyRoutingTable=true;           
                        }
         /*               {
                              NS_LOG_UNCOND(receiver<< " WWOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOW"); 
                              return;
                        }*/
                        if(myRoutingEntry.GetLifeTime ()<=Seconds(0))
                        {
                               modifyRoutingTable=true;   
        //                       NS_LOG_UNCOND(receiver<< " RT3 ROUTE EXPIRE");        
                        }              

                }
                else
                {
                        modifyRoutingTable=true;                       
                }


               if(modifyRoutingTable)  /// modify the route
                {
                     if(m_routingTableDown.LookupRoute (origin, myRoutingEntry))
                     {
                          m_routingTableDown.DeleteRoute(origin);
                     }


                      RoutingTableEntryDown newEntry (/*device=*/ dev, /*dst=*/ origin,/*seqNo=*/ srvadvertiseHeader.GetOriginSeqno (),
                                                              /*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0), /*hops=*/ hop,
                                                              /*nextHop*/ sink, /*timeLife=*/ m_activeRouteTimeout);
                      m_routingTableDown.AddRoute (newEntry);

                }


/* NS_LOG_UNCOND("Server routingtable RT3: ");
m_routingTableDown.PrintInScreen();*/

        return;
   
  }


}

void
RoutingProtocol::HelloTimerExpire2 () //DMS
{
  
  
if(!m_isServer)
{
  SendHello2 ();
}
  m_hellotimer.Cancel ();
  m_hellotimer.Schedule (m_helloTimerExpire);

}
void
RoutingProtocol::AdvertiseTimerExpire () //DMS
{
  
  
if(!m_isSink) // if this node has not an output access it can send an advertise message
{
  SendAdvertise ();
}
else
{
SendSrvAdvertise_testUDP();

}
  m_advertisetimer.Cancel ();
  m_advertisetimer.Schedule (m_advertiseTimerExpire);

/*
if((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.2"))
{
 NS_LOG_UNCOND("NODE RT2: "<<(m_socketAddresses.begin()->second).GetLocal ()<<"   "<<Simulator::Now ());
m_routingTableUp.PrintInScreen();
 NS_LOG_UNCOND("NODE RT3: "<<(m_socketAddresses.begin()->second).GetLocal ()<<"is valid?:");
m_routingTableDown.PrintInScreen();
}  */



}

void
RoutingProtocol::SendHello2 ()   //DMS
{
 Ptr<EnergySource> EnergySrc;
int i=0;//dms asupp
  NS_LOG_FUNCTION (this);
      RoutingTableEntryUp myRoutingEntry;
      EnergySrc = this->GetObject<EnergySourceContainer> ()->Get (0);
      m_minEnergy=EnergySrc->GetRemainingEnergy ();   
 
 if(m_isSink)
 {

Ipv4InterfaceAddress iface;
Ipv4Address ipv4ifaceSink;


  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {i++;//dms asupp
      Ptr<Socket> socket = j->first;
       iface = j->second;
  if (m_sinkOutPutDevice !=m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()))) 
    {
//  NS_LOG_UNCOND("remaining energy for sink is: "<<EnergySrc->GetRemainingEnergy ()*100);
      HelloHeader helloHeader (/*prefix size= 0,*/ /*hops=*/ 0, /* reserved*/ 0,/* Min Energy*/ m_minEnergy,/* Min snr*/ 9999.0, /*orig=sink*/ iface.GetLocal (), /*seqno=*/ m_HseqNo);
      Ptr<Packet> packet = Create<Packet> ();
      SocketIpTtlTag tag;
      tag.SetTtl (1);
      packet->AddPacketTag (tag);

      packet->AddHeader (helloHeader);
      TypeHeader tHeader (DMSRPTYPE_HELLO);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
      Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, destination);
   }
    }

   m_HseqNo++;

   return;
 }
 else // is simple node    //---------------------------simple node----------------------------
 {
   
   if(  m_routingTableUp.LookupBestRoute(myRoutingEntry))
   {
             EnergySrc = this->GetObject<EnergySourceContainer> ()->Get (0);
              m_minEnergy=EnergySrc->GetRemainingEnergy ();

        Ipv4InterfaceAddress iface;
        Ipv4Address ipv4ifaceSink;
        //find the best entry to create its hello header

          for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
            {i++;//dms asupp
              Ptr<Socket> socket = j->first;
               iface = j->second;
        //  NS_LOG_UNCOND("remaining energy for sink is: "<<EnergySrc->GetRemainingEnergy ()*100);
              HelloHeader helloHeader (/*prefix size= 0,*/ /*hops=*/ myRoutingEntry.GetHop(), /* reserved*/ 0,/* Min Energy*/ std::min (myRoutingEntry.GetCumEnergy(),(float)m_minEnergy),/* Min snr*/ myRoutingEntry.GetMinSnr(), /*sink=*/ myRoutingEntry.GetDestination (), /*seqno=*/ myRoutingEntry.GetSeqNo());
              Ptr<Packet> packet = Create<Packet> ();
              SocketIpTtlTag tag;
              tag.SetTtl (1);
              packet->AddPacketTag (tag);

              packet->AddHeader (helloHeader);
              TypeHeader tHeader (DMSRPTYPE_HELLO);
              packet->AddHeader (tHeader);
              // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
              Ipv4Address destination;
              if (iface.GetMask () == Ipv4Mask::GetOnes ())
                {
                  destination = Ipv4Address ("255.255.255.255");
                }
              else
                {
                  destination = iface.GetBroadcast ();
                }
              Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
              Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, destination);
          // NS_LOG_UNCOND("cumul energy for sink is: "<<helloHeader.GetMinEnergy ()*100);
           
            }

           return;
    }else
    {
           return;
    }

 }
}

void
RoutingProtocol::SendAdvertise ()   //DMS
{

  int i=0;//dms asupp
 if(m_isServer)
 {
   return;
 }
else
{
  NS_LOG_FUNCTION (this);

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {i++;//dms asupp
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      AdvertiseHeader advertiseHeader ( /*hops=*/ 0, /*prefix size=*/ 0, /*orig=*/ iface.GetLocal (), /*seqno=*/ m_AseqNo);
      Ptr<Packet> packet = Create<Packet> ();
      SocketIpTtlTag tag;
      tag.SetTtl (1);
      packet->AddPacketTag (tag);
      packet->AddHeader (advertiseHeader);
      TypeHeader tHeader (DMSRPTYPE_ADVERTISE);
      packet->AddHeader (tHeader);

      RoutingTableEntryUp RtoSink;
      m_routingTableUp.LookupBestRoute(RtoSink);
      if((!m_routingTableUp.IsEmpty()))                         //  && RtoSink.GetFlag () == VALID
      {
        Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
        Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, RtoSink.GetNextHop () );          
      }
    }

m_AseqNo++;
}
}


void
RoutingProtocol::SendSrvAdvertise_testUDP ()   //DMS   a supprimer
{
  Ptr<Ipv4Route> route=Create<Ipv4Route>();
  RoutingTableEntryUp toDst;


 if(m_isServer)
 {
   return;
 }
else
{

    if(!m_isSink)
    {
        return;
    }
    else
    {

 Ipv4InterfaceAddress wifiIface ;
 bool wifiIfaceExsit=false; 
          for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
            {
                      Ptr<Socket> socket = j->first;
                      Ipv4InterfaceAddress iface = j->second;
             //       NS_LOG_UNCOND ("ALL"<< iface.GetLocal ());
                    if (m_sinkOutPutDevice !=m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()))) 
                    {
                        wifiIfaceExsit=true;
                        wifiIface=iface;
           //     NS_LOG_UNCOND ("FFFFFFFFFFFFFFFFFFFF"<< wifiIface.GetLocal ());
                    }
            }

      for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
      {
           //   NS_LOG_UNCOND ("soc adresse: " <<i);
              Ptr<Socket> socket = j->first;
              Ipv4InterfaceAddress iface = j->second;


         if (wifiIfaceExsit &&(m_sinkOutPutDevice ==m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ())))) 
         {
          NS_LOG_FUNCTION (this);
                        m_routingTableUp.LookupBestRoute(toDst);
                        route = toDst.GetRoute ();
                        
              SrvAdvertiseHeader srvadvertiseHeader ( /*hops=*/ 0,/*prefix size=*/ 0, /*orig=*/ wifiIface.GetLocal (), /*sink=*/ iface.GetLocal (),/*seqno=*/ m_AseqNo);
              Ptr<Packet> packet = Create<Packet> ();
              SocketIpTtlTag tag;
              tag.SetTtl (1);
              packet->AddPacketTag (tag);
              packet->AddHeader (srvadvertiseHeader);
              TypeHeader tHeader (DMSRPTYPE_SRVADVERTISE);
              packet->AddHeader (tHeader);



                Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
                Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, m_serverAdress );
         }
        }
             

     }

m_AseqNo++;
}
}


void
RoutingProtocol::ForwardHello2 ( HelloHeader helloHeader)   //DMS
{
int i=0;//dms asupp
  NS_LOG_FUNCTION (this);
 Ptr<EnergySource> EnergySrc;
      EnergySrc = this->GetObject<EnergySourceContainer> ()->Get (0);
if(((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.8"))||((m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.17")))
{
 // NS_LOG_UNCOND ((m_socketAddresses.begin()->second).GetLocal ()<< "BATERY remaining enrgy: "<<EnergySrc->GetRemainingEnergy ()<<" cumul enrgy: "<<m_minEnergy); //DMS2

}


  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {i++;//dms asupp
   //   NS_LOG_UNCOND ("soc adresse: " <<i);
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
    //  HelloHeader helloHeader (/*prefix size=*/ 0, /*hops=*/ 0, /*HELLO ID=*/ m_helloId, /*orig=*/ iface.GetLocal (), /*seqno=*/ m_HseqNo);
      Ptr<Packet> packet = Create<Packet> ();
      SocketIpTtlTag tag;
      tag.SetTtl (1);
      packet->AddPacketTag (tag);


      packet->AddHeader (helloHeader);
      TypeHeader tHeader (DMSRPTYPE_HELLO);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
      Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, destination);

    }
}


void
RoutingProtocol::ForwardAdvertise ( AdvertiseHeader advertiseHeader)   //DMS   forwarding from simple node to anothor node or to sink 
{


int i=0;//dms asupp
  NS_LOG_FUNCTION (this);


  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {i++;//dms asupp
      Ptr<Socket> socket = j->first;
      Ptr<Packet> packet = Create<Packet> ();
      SocketIpTtlTag tag;
      tag.SetTtl (1);
      packet->AddPacketTag (tag);
      packet->AddHeader (advertiseHeader);
      TypeHeader tHeader (DMSRPTYPE_ADVERTISE);
      packet->AddHeader (tHeader);

     RoutingTableEntryUp RtoSink;
     m_routingTableUp.LookupBestRoute(RtoSink);
        Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
        if(m_isSink)
        { 
          return;//Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, m_serverAdress );   
        }
        else
        { 
           if((!m_routingTableUp.IsEmpty()))
            {
               Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, RtoSink.GetNextHop () ); 
            }  
        }
      
     }

}


void
RoutingProtocol::ForwardSrvAdvertise ( SrvAdvertiseHeader srvadvertiseHeader)   //DMS    Forward srvadvertiseHeader fby the sink to the server //this function is used only by a sink
{

  Ptr<Ipv4Route> route=Create<Ipv4Route>();
  RoutingTableEntryUp toDst;


 if(m_isServer)
 {
   return;
 }
else
{

    if(!m_isSink)
    {
        return;
    }
    else
    {
          for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
            {
           //   NS_LOG_UNCOND ("soc adresse: " <<i);
              Ptr<Socket> socket = j->first;
              Ipv4InterfaceAddress iface = j->second;
              //  socket->BindToNetDevice(m_sinkOutPutDevice);
              if (m_ipv4->GetInterfaceForDevice (m_sinkOutPutDevice)==m_ipv4->GetInterfaceForAddress (iface.GetLocal ()))
              {
   
                          NS_LOG_FUNCTION (this);
                                        m_routingTableUp.LookupBestRoute(toDst);
                                        route = toDst.GetRoute ();

                              Ptr<Packet> packet = Create<Packet> ();
                              SocketIpTtlTag tag;
                              tag.SetTtl (1);
                              packet->AddPacketTag (tag);
                              packet->AddHeader (srvadvertiseHeader);
                              TypeHeader tHeader (DMSRPTYPE_SRVADVERTISE);
                              packet->AddHeader (tHeader);
 
                                Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
                                Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this, socket, packet, m_serverAdress );                    
             }
          }
     }
m_AseqNo++;
}
}

Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}

Ptr<Socket>
RoutingProtocol::FindSubnetBroadcastSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketSubnetBroadcastAddresses.begin (); j != m_socketSubnetBroadcastAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}

/// Trace function for handle energy depletion.   //DMS
void
RoutingProtocol::EnergyDepletion (void) 
{
  //NS_LOG_UNCOND ("Energy depletion WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW   "<<Simulator::Now ());
}
void
RoutingProtocol::DepletionHandler (void)                                            // callback function for energy depletion
{

m_depTimeTrace();
  //NS_LOG_UNCOND ("Energy depletion"<<Simulator::Now ());
  //  NS_LOG_UNCOND ("Energy depletion handler   "<<Simulator::Now ());
   // DoDispose();                                                                   // if the energy source is drained -> the routing protocol stop functioning
}

void
RoutingProtocol::DoInitialize (void)
{

Ipv4InterfaceAddress iface;
Ipv4InterfaceAddress iface2;

RngSeedManager::SetRun(1);
RngSeedManager::SetSeed(m_rngRep);

Ptr<UniformRandomVariable>   m_uniformRandomVariable = CreateObject<UniformRandomVariable> (); //already declared
double rand; //already declared

Ptr<NetDevice> wifiNetDevice;// DMS
m_routingTableUp.SetRoutingMode (m_routingMode);

if(m_isServer)
{
  //NS_LOG_UNCOND((m_socketAddresses.begin()->second).GetLocal ()<<" :is SERVER"<<" NBr of Interfces :"<<m_ipv4->GetNInterfaces());
}

else
{   

   wifiNetDevice=m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress((m_socketAddresses.begin ()->second).GetLocal ()));

  if(m_isSink)
  {
  RoutingTableEntryUp rt;

//NS_LOG_UNCOND("Address of output device is: " <<m_sinkOutPutDevice->GetAddress());

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
       iface = j->second; 
    if (m_sinkOutPutDevice ==m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()))) 
      {
        //NS_LOG_UNCOND(" :is a SINK  : "<<(j->second).GetLocal ());
        rt.SetSource(j->second.GetLocal ());    
        //NS_LOG_UNCOND("the inetrface for device is: " <<m_ipv4->GetInterfaceForDevice (m_sinkOutPutDevice));
        rt.SetInterface(iface);
        iface2=iface;

      }
        else
        {
                wifiNetDevice=m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
        }
          //NS_LOG_UNCOND("itfce: " <<j->second << "intfces: "<<m_ipv4->GetInterfaceForAddress (iface.GetLocal ())<<" OUT DEV "<<m_sinkOutPutDevice<< "DEV : "<<m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (j->second.GetLocal ())));
    }


      m_routingTableUp.Clear();

      //Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));
      RoutingTableEntryUp newEntry (/*device=*/ m_sinkOutPutDevice, /*dst=*/ m_serverAdress,/*seqNo=*/ 0,
                                              /*iface=*/ iface2, /*hops=*/ 0,
                                              /*nextHop*/ m_sinkGateWayAddress, /*timeLife=*/ m_activeRouteTimeout*1000,99999,0);
      m_routingTableUp.AddRoute (newEntry);

      //NS_LOG_UNCOND("roooouuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuute table");
   m_routingTableUp.PrintInScreen();

    //NS_LOG_UNCOND(" :is a SINK   its server is: "<<m_serverAdress<< " Gateway adr: "<<m_sinkGateWayAddress << " device sink : "<< m_sinkOutPutDevice->GetAddress() <<" NBr of Interfces :"<<m_ipv4->GetNInterfaces());

  }

    m_hellotimer.SetFunction (&RoutingProtocol::HelloTimerExpire2, this);//DMS
          rand=m_uniformRandomVariable->GetValue(0,m_helloTimerExpire.GetSeconds());
    m_hellotimer.Schedule (Seconds(rand));// DMS

 m_advertisetimer.SetFunction (&RoutingProtocol::AdvertiseTimerExpire, this);//DMS
          rand=m_uniformRandomVariable->GetValue(0,m_advertiseTimerExpire.GetSeconds());
 m_advertisetimer.Schedule (Seconds(rand));// DMS

}

  NS_LOG_FUNCTION (this);

if(m_isSink/*(m_socketAddresses.begin()->second).GetLocal ()==Ipv4Address("10.1.0.2")*/)
{
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {//dms asupp
      Ptr<Socket> socket = j->first;
       iface = j->second;
              //NS_LOG_UNCOND("interface of the sink : "<<m_ipv4->GetInterfaceForAddress (iface.GetLocal ())<<"   ifce: "<<iface.GetLocal ());
    }
}


if(!m_isServer)
{
  Ptr<Node> nodeWifi = wifiNetDevice->GetNode();// NS_LOG_UNCOND("step 1");

  // source helper
  //  BasicEnergySourceHelper basicSourceHelper; //----------------------
  // set energy to 0 so that we deplete energy at the beginning of simulation
  //basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (15.0)); NS_LOG_UNCOND("step 2");//----------------------
  // set update interval
 // EnergySourceContainer energySource = basicSourceHelper.Install (nodeWifi); NS_LOG_UNCOND("step 3"); //--------------------
  Ptr<EnergySource> energySource = this->GetObject<EnergySourceContainer> ()->Get (0);
  // device energy model helper
  WifiRadioEnergyModelHelper radioEnergyHelper;
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0147));
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0097));
  radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue (0.000426));
//  radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue (0.0));
//  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.0));
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.000426));
  // set energy depletion callback
  WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback = MakeCallback (&RoutingProtocol::DepletionHandler, this);  //NS_LOG_UNCOND("step 4");
  radioEnergyHelper.SetDepletionCallback (callback); //NS_LOG_UNCOND("step 5");
  // install on node
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (wifiNetDevice, energySource);//NS_LOG_UNCOND("step 6");



  Ptr<EnergySource> EnergySrc = this->GetObject<EnergySourceContainer> ()->Get (0);
  //NS_LOG_UNCOND("remaining energy for sink is: "<<EnergySrc->GetRemainingEnergy ()*100);
  m_minEnergy=0; 
}

m_Snr=0;

  Ipv4RoutingProtocol::DoInitialize ();

}

} //namespace dmsrp
} //namespace ns3
