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
#ifndef DMSRPROUTINGPROTOCOL_H
#define DMSRPROUTINGPROTOCOL_H

#include "dmsrp-rtable.h"
#include "dmsrp-packet.h"
#include "ns3/snr-tag.h"     //DMS
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/traced-callback.h"
#include <map>

namespace ns3 {
namespace dmsrp {
/**
 * \ingroup dmsrp
 *
 * \brief DMSRP routing protocol
 */



class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  static const uint32_t DMSRP_PORT;

  /// constructor
  RoutingProtocol ();
  virtual ~RoutingProtocol ();
  virtual void DoDispose ();

  // Inherited from Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
 void EnergyDepletion (void);
  void DepletionHandler (void);
  // Handle protocol parameters


  /**
   * Set broadcast enable flag
   * \param f enable broadcast flag
   */
  void SetBroadcastEnable (bool f)
  {
    m_enableBroadcast = f;
  }
  /**
   * Get broadcast enable flag
   * \returns the broadcast enable flag
   */
  bool GetBroadcastEnable () const
  {
    return m_enableBroadcast;
  }

  void SetIsSink (bool f)     //DMS
  {
    m_isSink = f;
  }

bool GetIsSink ()   const  //DMS
  {
     return m_isSink;
  }

  void SetIsServer (bool f)     //DMS
  {
    m_isServer = f;
  }

bool GetIsServer ()  const   //DMS
  {
     return m_isServer;
  }

  void SetServerAddress (Ipv4Address f)     //DMS
  {
    m_serverAdress = f;
  }

Ipv4Address GetServerAddress ()  const   //DMS
  {
     return m_serverAdress;
  }


void SetSinkGateWayAddress (Ipv4Address f)     //DMS
  {
    m_sinkGateWayAddress = f;
  }

Ipv4Address GetSinkGateWayAddress ()  const   //DMS
  {
     return m_sinkGateWayAddress;
  }




  void Setm_SinkOutPutDevice (Ptr<NetDevice> f)     //DMS
  {
    m_sinkOutPutDevice = f;
  }

Ptr<NetDevice> Getm_SinkOutPutDevice ()  const   //DMS
  {
     return m_sinkOutPutDevice;
  }


  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  virtual void DoInitialize (void);
private:

  // Protocol parameters.
  Time m_activeRouteTimeout;          ///< Period of time during which the route is considered to be valid.
   
  bool m_enableBroadcast;              ///< Indicates whether a a broadcast data packets forwarding enable
  bool m_isSink; //DMS Has outside access? : should be initialized via the application
  bool m_isServer; //DMS this Node is a server? : should be initialized via the application
  Ipv4Address m_serverAdress; //DMS Serveradresse : should be initialized via the application
  Ipv4Address m_sinkGateWayAddress; //DMS Serveradresse : should be initialized via the application
  Ptr<NetDevice> m_sinkOutPutDevice;// DMS

  //\}

  /// IP protocol
  Ptr<Ipv4> m_ipv4;
  /// Raw unicast socket per each IP interface, map socket -> iface address (IP + mask)
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
  /// Raw subnet directed broadcast socket per each IP interface, map socket -> iface address (IP + mask)
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketSubnetBroadcastAddresses;
  /// Loopback device used to defer RREQ until packet will be fully formed
  Ptr<NetDevice> m_lo;

  uint32_t m_seqNo;

  /// Hello sequence number
  uint32_t m_HseqNo; //DMS
  /// Hello Advertise number
  uint32_t m_AseqNo; //DMS
  double temp_snr; //DMS
  float m_minEnergy; //DMS
  float m_Snr; //DMS
  RoutingMode m_routingMode; //DMS
  TracedCallback < > m_depTimeTrace; //DMS
  uint32_t m_rngRep; //DMS




private:
  /// Start protocol operation
  void Start ();
  /**
   * Queue packet and send route request
   *
   * \param p the packet to route
   * \param header the IP header
   * \param ucb the UnicastForwardCallback function
   * \param ecb the ErrorCallback function
   */ 
 // void DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);
  /**
   * If route exists and is valid, forward packet.
   *
   * \param p the packet to route
   * \param header the IP header
   * \param ucb the UnicastForwardCallback function
   * \param ecb the ErrorCallback function
   * \returns true if forwarded
   */ 
  bool Forwarding (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb, bool PcktFromServer);



  /**
   * Test whether the provided address is assigned to an interface on this node
   * \param src the source IP address
   * \returns true if the IP address is the node's IP address
   */
  bool IsMyOwnAddress (Ipv4Address src);
  /**
   * Find unicast socket with local interface address iface
   *
   * \param iface the interface
   * \returns the socket associated with the interface
   */
  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;
  /**
   * Find subnet directed broadcast socket with local interface address iface
   *
   * \param iface the interface
   * \returns the socket associated with the interface
   */
  Ptr<Socket> FindSubnetBroadcastSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;

  /**
   * Create loopback route for given header
   *
   * \param header the IP header
   * \param oif the output interface net device
   * \returns the route
   */
  Ptr<Ipv4Route> LoopbackRoute (const Ipv4Header & header, Ptr<NetDevice> oif) const;

  ///\name Receive control packets
  //\{
  /// Receive and process control packet
  void RecvDmsrp (Ptr<Socket> socket);
  /// Receive HELLO
  void RecvHello (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src);//DMS
  /// Receive ADVERTISE
  void RecvAdvertise (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src);//DMS
  /// Receive SRVADVERTISE
  void RecvSrvAdvertise (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src);//DMS


  void SendHello2 (); //DMS
  void SendAdvertise (); //DMS   send advertise packet from simple nodes to sink
 void SendSrvAdvertise_testUDP (); //DMS    send advertise packet from sink to server

  void SendToServer(Ptr<Packet> p, Ipv4Address serv); //DMS
  void ForwardHello2(HelloHeader helloHeader); //DMS
  void ForwardAdvertise(AdvertiseHeader advertiseHeader); //DMS
  void ForwardSrvAdvertise(SrvAdvertiseHeader srvadvertiseHeader); //DMS   forward advertise packet received by the sink to the server


  /**
   * Send packet to desitnation scoket
   * \param socket - destination node socket
   * \param packet - packet to send
   * \param destination - destination node IP address
   */
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);

  /// Hello timer
  Timer m_hellotimer; //DMS
  /// Hello timer
  Timer m_advertisetimer; //DMS

  /// Schedule next send of hello message
  void HelloTimerExpire2 ();//DMS
  /// Schedule next send of hello message
  void AdvertiseTimerExpire ();//DMS











  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /// Routing table                  //DMS
  RoutingTableUp m_routingTableUp;       //DMS
  RoutingTableDown m_routingTableDown;       //DMS  
  uint32_t m_maxHelloHops; //DMS 
  uint32_t m_maxAdvertiseHops; //DMS 
  Time m_helloTimerExpire; //DMS 
  Time m_advertiseTimerExpire; //DMS
  double m_snrWeight; //DMS
  double m_energyWeight; //DMS
};

} //namespace dmsrp
} //namespace ns3

#endif /* DMSRPROUTINGPROTOCOL_H */
