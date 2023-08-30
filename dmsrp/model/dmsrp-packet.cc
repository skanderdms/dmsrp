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
#include "dmsrp-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3 {
namespace dmsrp {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dmsrp::TypeHeader")
    .SetParent<Header> ()
    .SetGroupName ("Dmsrp")
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
    case DMSRPTYPE_HELLO://DMS
    case DMSRPTYPE_ADVERTISE://DMS  
    case DMSRPTYPE_SRVADVERTISE://DMS 
      {
        m_type = (MessageType) type;
        break;
      }
    default:
      m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case DMSRPTYPE_HELLO://DMS
      {
        os << "HELLO";
        break;
      }
    case DMSRPTYPE_ADVERTISE://DMS
      {
        os << "ADVERTISE";
        break;
      }
    case DMSRPTYPE_SRVADVERTISE://DMS
      {
        os << "SRVADVERTISE";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}


//-----------------------------------------------------------------------------
// HELLO
//-----------------------------------------------------------------------------
HelloHeader::HelloHeader (uint8_t hopCount, uint16_t reserved,float minEnergy,float minSnr, Ipv4Address origin, uint32_t HSeqNo)
  : m_hopCount (hopCount),
    m_reserved (reserved),
    m_minEnergy (minEnergy),
    m_minSnr (minSnr),
    m_origin (origin),
    m_HseqNo (HSeqNo)
{
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dmsrp::HelloHeader")
    .SetParent<Header> ()
    .SetGroupName ("Dmsrp")
    .AddConstructor<HelloHeader> ()
  ;
  return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
HelloHeader::GetSerializedSize () const
{
  return 19;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
uu_32 tmp32;
  i.WriteU8 (m_hopCount);
  i.WriteU16 (m_reserved);
  //i.WriteU32 (m_minEnergy);
  tmp32.f=m_minEnergy;
  i.WriteHtonU32 ((uint32_t)tmp32.i);
  tmp32.f=m_minSnr;
  i.WriteHtonU32 ((uint32_t)tmp32.i);
    WriteTo (i, m_origin);
  i.WriteHtonU32 (m_HseqNo);
}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{

uu_32 tmp32;
  Buffer::Iterator i = start;
  m_hopCount = i.ReadU8 ();
  m_reserved = i.ReadU16 ();
//  m_minEnergy = i.ReadU32 ();
  tmp32.i=i.ReadNtohU32 ();
  m_minEnergy = (float) tmp32.f;
  tmp32.i=i.ReadNtohU32 ();
  m_minSnr = (float) tmp32.f;
  ReadFrom (i, m_origin);
  m_HseqNo = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << " destination: ipv4 " << "DMS Out"
     << " sequence number " << m_HseqNo << " source: ipv4 "
     << m_origin;
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
  h.Print (os);
  return os;
}

bool
HelloHeader::operator== (HelloHeader const & o) const
{
  return (m_hopCount == o.m_hopCount && m_reserved == o.m_reserved && m_minEnergy == o.m_minEnergy && m_minSnr == o.m_minSnr
          && m_origin == o.m_origin && m_HseqNo == o.m_HseqNo);
}



//-----------------------------------------------------------------------------
// ADVERTISE
//-----------------------------------------------------------------------------
AdvertiseHeader::AdvertiseHeader ( uint8_t hopCount, uint16_t reserved,Ipv4Address origin, uint32_t ASeqNo)
  :  m_hopCount (hopCount),
     m_reserved (reserved),
     m_origin (origin),
     m_AseqNo (ASeqNo)
{
}

NS_OBJECT_ENSURE_REGISTERED (AdvertiseHeader);

TypeId
AdvertiseHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dmsrp::AdvertiseHeader")
    .SetParent<Header> ()
    .SetGroupName ("Dmsrp")
    .AddConstructor<AdvertiseHeader> ()
  ;
  return tid;
}

TypeId
AdvertiseHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
AdvertiseHeader::GetSerializedSize () const
{
  return 11;
}

void
AdvertiseHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_hopCount);
  i.WriteU16 (m_reserved);
    WriteTo (i, m_origin);
  i.WriteHtonU32 (m_AseqNo);
}

uint32_t
AdvertiseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_hopCount = i.ReadU8 ();
  m_reserved = i.ReadU16 ();
  ReadFrom (i, m_origin);
  m_AseqNo = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
AdvertiseHeader::Print (std::ostream &os) const
{
  os << " destination: ipv4 " << "DMS Out"
     << " sequence number " << m_AseqNo << " source: ipv4 "
     << m_origin;
}

std::ostream &
operator<< (std::ostream & os, AdvertiseHeader const & h)
{
  h.Print (os);
  return os;
}

bool
AdvertiseHeader::operator== (AdvertiseHeader const & o) const
{
  return ( m_hopCount == o.m_hopCount && m_reserved == o.m_reserved
          && m_origin == o.m_origin && m_AseqNo == o.m_AseqNo);
}




//-----------------------------------------------------------------------------
// SRVADVERTISE
//-----------------------------------------------------------------------------
SrvAdvertiseHeader::SrvAdvertiseHeader ( uint8_t hopCount, uint16_t reserved, Ipv4Address origin,Ipv4Address sink, uint32_t ASeqNo)
  : m_hopCount (hopCount),
    m_reserved (reserved),
    m_origin (origin),
    m_sink (sink),
    m_AseqNo (ASeqNo)
{
}

NS_OBJECT_ENSURE_REGISTERED (SrvAdvertiseHeader);

TypeId
SrvAdvertiseHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dmsrp::SrvAdvertiseHeader")
    .SetParent<Header> ()
    .SetGroupName ("Dmsrp")
    .AddConstructor<SrvAdvertiseHeader> ()
  ;
  return tid;
}

TypeId
SrvAdvertiseHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
SrvAdvertiseHeader::GetSerializedSize () const
{
  return 15;
}

void
SrvAdvertiseHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_hopCount);
  i.WriteU16 (m_reserved);
    WriteTo (i, m_origin);
    WriteTo (i, m_sink);
  i.WriteHtonU32 (m_AseqNo);
}

uint32_t
SrvAdvertiseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_hopCount = i.ReadU8 ();
  m_reserved = i.ReadU16 ();
  ReadFrom (i, m_origin);
  ReadFrom (i, m_sink);
  m_AseqNo = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
SrvAdvertiseHeader::Print (std::ostream &os) const
{
  os << " destination: ipv4 " << "DMS Out"
     << " sequence number " << m_AseqNo << " source: ipv4 " << m_origin  << " sink: ipv4 " << m_sink;
}

std::ostream &
operator<< (std::ostream & os, SrvAdvertiseHeader const & h)
{
  h.Print (os);
  return os;
}

bool
SrvAdvertiseHeader::operator== (SrvAdvertiseHeader const & o) const
{
  return ( m_hopCount == o.m_hopCount && m_reserved == o.m_reserved
          && m_origin == o.m_origin && m_sink == o.m_sink && m_AseqNo == o.m_AseqNo);
}


}
}
