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
#ifndef DMSRP_RTABLE_H
#define DMSRP_RTABLE_H

#include <stdint.h>
#include <cassert>
#include <map>
#include <sys/types.h>
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {
namespace dmsrp {


enum RoutingMode
{
  BASIC_MODE = 0,          //!< 
  MULTI_PARENT_MODE = 1,      //!< 
  ENERGY_AWARE_MULTI_PARENT_MODE = 2,      //!< 
  SNR_AWARE_MULTI_PARENT_MODE = 3,      //!< 
};

// DMS RoutingTableEntryUp ---------------------------------------
/**
 * \ingroup dmsrp
 * \brief Routing table entry
 */
class RoutingTableEntryUp
{
public:
  /**
   * constructor
   *
   * \param dev the device
   * \param dst the destination IP address
   * \param vSeqNo verify sequence number flag
   * \param seqNo the sequence number
   * \param iface the interface
   * \param hops the number of hops
   * \param nextHop the IP address of the next hop
   * \param lifetime the lifetime of the entry
   */
  RoutingTableEntryUp (Ptr<NetDevice> dev = 0, Ipv4Address dst = Ipv4Address (), uint32_t seqNo = 0,
                     Ipv4InterfaceAddress iface = Ipv4InterfaceAddress (), uint16_t  hops = 0, Ipv4Address nextHop = Ipv4Address (), Time lifetime = Simulator::Now (),float cumEnergy=0,float minSnr=99999);

  ~RoutingTableEntryUp ();


  Ipv4Address GetDestination () const
  {
    return m_ipv4Route->GetDestination ();
  }
  /**
   * Get route function
   * \returns The IPv4 route
   */
  Ptr<Ipv4Route> GetRoute () const
  {
    return m_ipv4Route;
  }
  /**
   * Set route function
   * \param r the IPv4 route
   */
  void SetRoute (Ptr<Ipv4Route> r)
  {
    m_ipv4Route = r;
  }
  /**
   * Set next hop address
   * \param nextHop the next hop IPv4 address
   */
  void SetNextHop (Ipv4Address nextHop)
  {
    m_ipv4Route->SetGateway (nextHop);
  }
  /**
   * Get next hop address
   * \returns the next hop address
   */
  Ipv4Address GetNextHop () const
  {
    return m_ipv4Route->GetGateway ();
  }
  /**
   * Set output device
   * \param dev The output device
   */
  void SetOutputDevice (Ptr<NetDevice> dev)
  {
    m_ipv4Route->SetOutputDevice (dev);
  }
  /**
   * Get output device
   * \returns the output device
   */
  Ptr<NetDevice> GetOutputDevice () const
  {
    return m_ipv4Route->GetOutputDevice ();
  }

  /**

   */
  void SetSource (Ipv4Address src)
  {
    m_ipv4Route->SetSource (src);
  }
  /**

   */
  Ipv4Address GetSource () const
  {
    return m_ipv4Route->GetSource ();
  }

  /**

   */
  void SetDestination (Ipv4Address dst)
  {
    m_ipv4Route->SetDestination (dst);
  }

  /**

   */
  void SetGateway (Ipv4Address gtw)
  {
    m_ipv4Route->SetGateway (gtw);
  }
  /**

   */
  Ipv4Address GetGateway () const
  {
    return m_ipv4Route->GetGateway ();
  }

  /**
   * Get the Ipv4InterfaceAddress
   * \returns the Ipv4InterfaceAddress
   */
  Ipv4InterfaceAddress GetInterface () const
  {
    return m_iface;
  }
  /**
   * Set the Ipv4InterfaceAddress
   * \param iface The Ipv4InterfaceAddress
   */
  void SetInterface (Ipv4InterfaceAddress iface)
  {
    m_iface = iface;
  }

  /**
   * Set the sequence number
   * \param sn the sequence number
   */
  void SetSeqNo (uint32_t sn)
  {
    m_seqNo = sn;
  }
  /**
   * Get the sequence number
   * \returns the sequence number
   */
  uint32_t GetSeqNo () const
  {
    return m_seqNo;
  }
  /**
   * Set the number of hops
   * \param hop the number of hops
   */
  void SetHop (uint16_t hop)
  {
    m_hops = hop;
  }
  /**
   * Get the number of hops
   * \returns the number of hops
   */
  uint16_t GetHop () const
  {
    return m_hops;
  }
  /**
   * Set the lifetime
   * \param lt The lifetime
   */
  void SetLifeTime (Time lt)
  {
    m_lifeTime = lt + Simulator::Now ();
  }
  /**
   * Get the lifetime
   * \returns the lifetime
   */
  Time GetLifeTime () const
  {
    return m_lifeTime - Simulator::Now ();
  }
  /**
   * Set the minSnr
   * \param lt The minSnr
   */
  void SetMinSnr (float minSnr)
  {
    m_minSnr = minSnr;
  }
  /**
   * Get the minSnr
   * \param lt The minSnr
   */
  float GetMinSnr () const
  {
    return m_minSnr;
  }

  /**
   * Set the m_cumEnergy
   * \param lt The m_cumEnergy
   */
  void SetCumEnergy (float minSnr)
  {
    m_minSnr = minSnr;
  }
  /**
   * Get the m_cumEnergy
   * \param lt The m_cumEnergy
   */
  float GetCumEnergy () const
  {
    return m_cumEnergy;
  }


  /// RREP_ACK timer
  Timer m_ackTimer;

  /**
   * \brief Compare destination address
   * \param dst IP address to compare
   * \return true if equal
   */
 bool operator== (Ipv4Address const  dst) const
  {
    return (m_ipv4Route->GetDestination () == dst);
  }
  /**
   * Print packet to trace file
   * \param stream The output stream
   */
  void Print (Ptr<OutputStreamWrapper> stream) const;
   void PrintInScreen () const; //DMS added by dms

private:

  /// Destination Sequence Number, if m_validSeqNo = true
  uint32_t m_seqNo;
  /// Hop Count (number of hops needed to reach destination)
  uint16_t m_hops;                //DMS IF IT IS A SERVER this field hs no role
  /**
  */
  Time m_lifeTime;
  /** Ip route, include
   *   - destination address
   *   - source address
   *   - next hop address (gateway)
   *   - output device
   */
float m_cumEnergy;
float m_minSnr;
  Ptr<Ipv4Route> m_ipv4Route;
  /// Output interface address
  Ipv4InterfaceAddress m_iface;

};


// end DMS RoutingTableEntryUp ---------------------------------------
/**
 * \ingroup dmsrp
 * \brief Routing table entry
 */


// DMS RoutingTableEntryDown ---------------------------------------
/**
 * \ingroup dmsrp
 * \brief Routing table entry
 */
class RoutingTableEntryDown
{
public:
  /**
   * constructor
   *
   * \param dev the device
   * \param dst the destination IP address
   * \param vSeqNo verify sequence number flag
   * \param seqNo the sequence number
   * \param iface the interface
   * \param hops the number of hops
   * \param nextHop the IP address of the next hop
   * \param lifetime the lifetime of the entry
   */
  RoutingTableEntryDown (Ptr<NetDevice> dev = 0, Ipv4Address dst = Ipv4Address (), uint32_t seqNo = 0,
                     Ipv4InterfaceAddress iface = Ipv4InterfaceAddress (), uint16_t  hops = 0, Ipv4Address nextHop = Ipv4Address (), Time lifetime = Simulator::Now ());

  ~RoutingTableEntryDown ();

 
  Ipv4Address GetDestination () const
  {
    return m_ipv4Route->GetDestination ();
  }
  /**
   * Get route function
   * \returns The IPv4 route
   */
  Ptr<Ipv4Route> GetRoute () const
  {
    return m_ipv4Route;
  }
  /**
   * Set route function
   * \param r the IPv4 route
   */
  void SetRoute (Ptr<Ipv4Route> r)
  {
    m_ipv4Route = r;
  }
  /**
   * Set next hop address
   * \param nextHop the next hop IPv4 address
   */
  void SetNextHop (Ipv4Address nextHop)
  {
    m_ipv4Route->SetGateway (nextHop);
  }
  /**
   * Get next hop address
   * \returns the next hop address
   */
  Ipv4Address GetNextHop () const
  {
    return m_ipv4Route->GetGateway ();
  }
  /**
   * Set output device
   * \param dev The output device
   */
  void SetOutputDevice (Ptr<NetDevice> dev)
  {
    m_ipv4Route->SetOutputDevice (dev);
  }
  /**
   * Get output device
   * \returns the output device
   */
  Ptr<NetDevice> GetOutputDevice () const
  {
    return m_ipv4Route->GetOutputDevice ();
  }

  /**

   */
  void SetSource (Ipv4Address src)
  {
    m_ipv4Route->SetSource (src);
  }
  /**

   */
  Ipv4Address GetSource () const
  {
    return m_ipv4Route->GetSource ();
  }

  /**

   */
  void SetDestination (Ipv4Address dst)
  {
    m_ipv4Route->SetDestination (dst);
  }
  /**

   */
  void SetGateway (Ipv4Address gtw)
  {
    m_ipv4Route->SetGateway (gtw);
  }
  /**

   */
  Ipv4Address GetGateway () const
  {
    return m_ipv4Route->GetGateway ();
  }
  /**
   * Get the Ipv4InterfaceAddress
   * \returns the Ipv4InterfaceAddress
   */
  Ipv4InterfaceAddress GetInterface () const
  {
    return m_iface;
  }
  /**
   * Set the Ipv4InterfaceAddress
   * \param iface The Ipv4InterfaceAddress
   */
  void SetInterface (Ipv4InterfaceAddress iface)
  {
    m_iface = iface;
  }
  /**
   * Set the sequence number
   * \param sn the sequence number
   */
  void SetSeqNo (uint32_t sn)
  {
    m_seqNo = sn;
  }
  /**
   * Get the sequence number
   * \returns the sequence number
   */
  uint32_t GetSeqNo () const
  {
    return m_seqNo;
  }
  /**
   * Set the number of hops
   * \param hop the number of hops
   */
  void SetHop (uint16_t hop)
  {
    m_hops = hop;
  }
  /**
   * Get the number of hops
   * \returns the number of hops
   */
  uint16_t GetHop () const
  {
    return m_hops;
  }
  /**
   * Set the lifetime
   * \param lt The lifetime
   */
  void SetLifeTime (Time lt)
  {
    m_lifeTime = lt + Simulator::Now ();
  }
  /**
   * Get the lifetime
   * \returns the lifetime
   */
  Time GetLifeTime () const
  {
    return m_lifeTime - Simulator::Now ();
  }

  /// RREP_ACK timer
  Timer m_ackTimer;

  /**
   * \brief Compare destination address
   * \param dst IP address to compare
   * \return true if equal
   */
 bool operator== (Ipv4Address const  dst) const
  {
    return (m_ipv4Route->GetDestination () == dst);
  }
  /**
   * Print packet to trace file
   * \param stream The output stream
   */
  void Print (Ptr<OutputStreamWrapper> stream) const;
   void PrintInScreen () const; //DMS added by dms

private:

  /// Destination Sequence Number, if m_validSeqNo = true
  uint32_t m_seqNo;
  /// Hop Count (number of hops needed to reach destination)
  uint16_t m_hops;                //DMS IF IT IS A SERVER this field hs no role
  /**
  * \brief Expiration or deletion time of the route
  *	Lifetime field in the routing table plays dual role:
  *	for an active route it is the expiration time, and for an invalid route
  *	it is the deletion time.
  */
  Time m_lifeTime;
  /** Ip route, include
   *   - destination address
   *   - source address
   *   - next hop address (gateway)
   *   - output device
   */
  Ptr<Ipv4Route> m_ipv4Route;
  /// Output interface address
  Ipv4InterfaceAddress m_iface;
};


// end DMS RoutingTableEntryDown ---------------------------------------
/**
 * \ingroup dmsrp
 * \brief Routing table entry
 */
//------------------------------------------------------
// DMS routing table2


/**
 * \ingroup dmsrp
 * \brief The Routing table used by DMSRP protocol
 */
class RoutingTableUp
{
public:
  /**
   * constructor
   * \param t the routing table entry lifetime
   */
  RoutingTableUp ();
 
  /**
   * Add routing table entry if it doesn't yet exist in routing table
   * \param r routing table entry
   * \return true in success
   */
  bool AddRoute (RoutingTableEntryUp & r);
  /**
   * Delete routing table entry with destination address dst, if it exists.
   * \param dst destination address
   * \return true on success
   */
  bool DeleteRoute (Ipv4Address dst);



  bool IsEmpty();  //DMS
  bool LookupTheRoute (RoutingTableEntryUp & rt);  //DMS added by dms
uint32_t GetMinHops ();   // DMS
  bool LookupBestRoute (RoutingTableEntryUp & rt);   // DMS
   bool GetNextNode (Ipv4Address & NextNodeAdr);  // DMS//get the IP adresse of the next node to keep the sink - DMS added by dms


  /**
   * Lookup routing table entry with destination address dst
   * \param dst destination address
   * \param rt entry with destination address dst, if exists
   * \return true on success
   */
  bool LookupRoute (Ipv4Address dst, RoutingTableEntryUp & rt);
 
  /**
   * Update routing table
   * \param rt entry with destination address dst, if exists
   * \return true on success
   */
 // bool Update (RoutingTableEntryUp & rt);


bool UpdateLifeTimeEntry (RoutingTableEntryUp & rt, Time lt) ; //ADDED BY DMS
 
   /**
   * Delete all route from interface with address iface
   * \param iface the interface IP address
   */
  void DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface);
  /// Delete all entries from routing table
  void Clear ()
  {
    m_ipv4AddressEntry.clear ();
  }
  /// Delete all outdated entries 
  void Purge ();

  /**
   * Set the routing mode
   * \param lt The m_routingMode
   */
  void SetRoutingMode (RoutingMode routingMode)
  {
    m_routingMode = routingMode;
  }
  /**
   * Get the routing mode
   * \param lt The m_routingMode
   */
  RoutingMode GetRoutingMode () const
  {
    return m_routingMode;
  }

   /**
   * Print routing table
   * \param stream the output stream
   */

  void Print (Ptr<OutputStreamWrapper> stream) const;
 void PrintInScreen () const ;// DMS added by dms

private:
  /// The routing table
  std::map<Ipv4Address, RoutingTableEntryUp> m_ipv4AddressEntry;
 
  /**
   * const version of Purge, for use by Print() method
   * \param table the routing table entry to purge
   */
  RoutingMode m_routingMode; //DMS
  void Purge (std::map<Ipv4Address, RoutingTableEntryUp> &table) const;
};

// end DMS routing table2   --------------------------------------------------------





//------------------------------------------------------
// DMS routing table3


/**
 * \ingroup dmsrp
 * \brief The Routing table used by DMSRP protocol
 */
class RoutingTableDown
{
public:
  /**
   * constructor
   * \param t the routing table entry lifetime
   */
  RoutingTableDown (Time t);
 
  /**
   * Add routing table entry if it doesn't yet exist in routing table
   * \param r routing table entry
   * \return true in success
   */
  bool AddRoute (RoutingTableEntryDown & r);
  /**
   * Delete routing table entry with destination address dst, if it exists.
   * \param dst destination address
   * \return true on success
   */
  bool DeleteRoute (Ipv4Address dst);



  bool IsEmpty();  //DMS

  /**
   * Lookup routing table entry with destination address dst
   * \param dst destination address
   * \param rt entry with destination address dst, if exists
   * \return true on success
   */
  bool LookupRoute (Ipv4Address dst, RoutingTableEntryDown & rt);
 

  bool UpdateLifeTimeEntry (RoutingTableEntryDown & rt, Time lt) ;  // added by DMS

  /**
   * Delete all route from interface with address iface
   * \param iface the interface IP address
   */
  void DeleteAllRoutesFromInterface (Ipv4InterfaceAddress iface);
  /// Delete all entries from routing table
  void Clear ()
  {
    m_ipv4AddressEntry.clear ();
  }
  /// Delete all outdated entries 
  void Purge ();

  /**
   * Print routing table
   * \param stream the output stream
   */
  void Print (Ptr<OutputStreamWrapper> stream) const;
 void PrintInScreen () const ;// DMS added by dms

private:
  /// The routing table
  std::map<Ipv4Address, RoutingTableEntryDown> m_ipv4AddressEntry;

  /**
   * const version of Purge, for use by Print() method
   * \param table the routing table entry to purge
   */
  void Purge (std::map<Ipv4Address, RoutingTableEntryDown> &table) const;
};

// end DMS routing table2   --------------------------------------------------------


}  // namespace dmsrp
}  // namespace ns3

#endif /* DMSRP_RTABLE_H */
