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
#ifndef DMSRPPACKET_H
#define DMSRPPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"

namespace ns3 {
namespace dmsrp {


typedef union{
  uint32_t i;
  float f;
 } uu_32;



/**
* \ingroup dmsrp
* \brief MessageType enumeration
*/
enum MessageType
{
  DMSRPTYPE_HELLO  = 1,   //!< DMSRPTYPE_HELLO//DMS
  DMSRPTYPE_ADVERTISE  = 2,   //!< DMSRPTYPE_ADVERTISE//DMS
  DMSRPTYPE_SRVADVERTISE  = 3   //!< DMSRPTYPE_SRVADVERTISE//DMS
};

/**
* \ingroup dmsrp
* \brief DMSRP types
*/
class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the DMSRP RREQ type
   */
  TypeHeader (MessageType t = DMSRPTYPE_HELLO);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /**
   * \returns the type
   */
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \returns true if the type is valid
   */
  bool IsValid () const
  {
    return m_valid;
  }
  /**
   * \brief Comparison operator
   * \param o header to compare
   * \return true if the headers are equal
   */
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, TypeHeader const & h);
// DMS *************************************************
/**
* \ingroup dmsrp
* \brief   Hello  Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |   Hop Count   |            Reserved           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Cum Energy                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            Min Snr                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
  |                     Originator IP Address                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Sequence Number                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
class HelloHeader : public Header  // DMS
{
public:
  /**
   * constructor
   *
   * \param flags the message flags (0)
   * \param reserved the reserved bits (0)
   * \param hopCount the hop count
   * \param helloID the hello ID
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param originSeqNo the origin sequence number
   */
   HelloHeader ( uint8_t hopCount = 0, uint16_t reserved = 0, float minEnergy = 0,float minSnr = 0, Ipv4Address origin = Ipv4Address (), uint32_t HSeqNo = 0);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  /**
   * \brief Set the hop count
   * \param count the hop count
   */
  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }

  /**
   * \brief Set the min energy
   * \param count the min nergy
   */
  void SetMinEnergy (float Energy)
  {
    m_minEnergy = Energy;
  }
  /**
   * \brief Get the min energy
   * \return the min energy
   */
  float GetMinEnergy () const
  {
    return m_minEnergy;
  }
  /**
   * \brief Set the min snr 
   * \param count the min snr
   */
  void SetMinSnr (float Snr)
  {
    m_minSnr = Snr;
  }
  /**
   * \brief Get the min snr
   * \return the min snr
   */
  float GetMinSnr () const
  {
    return m_minSnr;
  }


  /**
   * \brief Set the hello ID
   * \param id the hello ID
   */
/*  void SetId (uint32_t id)
  {
    m_helloID = id;
  }*/
  /**
   * \brief Get the hello ID
   * \return the hello ID
   */
 /* uint32_t GetId () const
  {
    return m_helloID;
  }*/
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }
  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  void SetSeqno (uint32_t s)
  {
    m_HseqNo = s;
  }
  /**
   * \brief Get the origin sequence number
   * \return the origin sequence number
   */
  uint32_t GetOriginSeqno () const
  {
    return m_HseqNo;
  }

   bool operator== (HelloHeader const & o) const;
private:

  uint8_t        m_hopCount;       ///< Hop Count
  uint16_t        m_reserved;       ///< Not used (must be 0)
  float        m_minEnergy;       ///< Not used (must be 0
  float        m_minSnr;       ///< Not used (must be 0

 // uint32_t       m_helloID;      ///< HELLO ID
  Ipv4Address    m_origin;         ///< Originator IP Address
  uint32_t       m_HseqNo;    ///< Source Sequence Number
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, HelloHeader const &);

// DMS ************************************************* ADVERTISE
/**
* \ingroup dmsrp
* \brief   Advertise  Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |   Hop Count   |            Reserved           |  
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Originator IP Address                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Sequence Number                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
class AdvertiseHeader : public Header  // DMS
{
public:
  /**
   * constructor
   *
   * \param flags the message flags (0)
   * \param reserved the reserved bits (0)
   * \param hopCount the hop count
   * \param advertiseID the advertise ID
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param originSeqNo the origin sequence number
   */
   AdvertiseHeader (  uint8_t hopCount = 0,uint16_t reserved = 0,
              Ipv4Address origin = Ipv4Address (),uint32_t ASeqNo = 0);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  /**
   * \brief Set the hop count
   * \param count the hop count
   */
  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }
 
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }
  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  void SetSeqno (uint32_t s)
  {
    m_AseqNo = s;
  }
  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  uint32_t GetSeqno ()
  {
    return m_AseqNo ;
  }
  /**
   * \brief Get the origin sequence number
   * \return the origin sequence number
   */
  uint32_t GetOriginSeqno () const
  {
    return m_AseqNo;
  }

   bool operator== (AdvertiseHeader const & o) const;
private:
  uint8_t        m_hopCount;       ///< Hop Count
  uint16_t        m_reserved;       ///< Not used (must be 0)
  Ipv4Address    m_origin;         ///< Originator IP Address
  uint32_t       m_AseqNo;    ///< Source Sequence Number
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, AdvertiseHeader const &);

// end ADVERTISE_SERVER header //////////////////////////////////////////////////////////





// DMS ************************************************* ADVERTISE
/**
* \ingroup dmsrp
* \brief   SRVAdvertise  Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |   Hop Count   |            Reserved           | 
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Originator IP Address                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                           Sequence Number                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
class SrvAdvertiseHeader : public Header  // DMS
{
public:
  /**
   * constructor
   *
   * \param flags the message flags (0)
   * \param reserved the reserved bits (0)
   * \param hopCount the hop count
   * \param advertiseID the advertise ID
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param originSeqNo the origin sequence number
   */
   SrvAdvertiseHeader (  uint8_t hopCount = 0,uint16_t reserved = 0, Ipv4Address origin = Ipv4Address (),
                        Ipv4Address sink = Ipv4Address (), uint32_t ASeqNo = 0);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields


  uint32_t  GetSize (void)
  {
    return 16;
  }
  /**
   * \brief Set the hop count
   * \param count the hop count
   */

  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }
 
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }


  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetSink(Ipv4Address a)//DMD
  {
    m_sink = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetSink () const  //DMS
  {
    return m_sink;
  }

  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  void SetSeqno (uint32_t s)
  {
    m_AseqNo = s;
  }
  /**
   * \brief Get the origin sequence number
   * \return the origin sequence number
   */
  uint32_t GetOriginSeqno () const
  {
    return m_AseqNo;
  }

   bool operator== (SrvAdvertiseHeader const & o) const;
private:
  uint8_t        m_hopCount;       ///< Hop Count
  uint16_t        m_reserved;       ///< Not used (must be 0)
  Ipv4Address    m_origin;         ///< Originator IP Address
  Ipv4Address    m_sink;         ///< Sink IP Address
  uint32_t       m_AseqNo;    ///< Source Sequence Number
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, SrvAdvertiseHeader const &);

// end ADVERTISESERVER header //////////////////////////////////////////////////////////



}  // namespace dmsrp
}  // namespace ns3

#endif /* DMSRPPACKET_H */
