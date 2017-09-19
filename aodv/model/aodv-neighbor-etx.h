/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
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
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 * 
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Nenad Jevtic <n.jevtic@sf.bg.ac.rs>
 *          Marija Malnar <m.malnar@sf.bg.ac.rs>
 */
 
#ifndef AODVNEIGHBORETX_H
#define AODVNEIGHBORETX_H

#include <map>
#include "ns3/ipv4-address.h"
#include "ns3/aodv-packet.h"

namespace ns3
{
namespace aodv
{

class NeighborEtx
{
public:
  NeighborEtx ();
  struct Etx
  {
    uint16_t m_lppMyCnt10bMap;
    uint8_t m_lppReverse;
    Etx () : m_lppMyCnt10bMap (0), m_lppReverse (0) {}
  };
  
  void GotoNextTimeStampAndClearOldest ();
  uint8_t GetLppTimeStamp () {return m_lppTimeStamp; }
  void FillLppCntData (LppHeader &lppHeader);
  bool Update (Ipv4Address addr, uint8_t lppTimeStamp, uint8_t lppReverse);
  uint32_t CalculateBinaryShiftedEtx (struct Etx etxStruct);
private:
  std::map<Ipv4Address, Etx> m_neighborEtx;
  uint8_t m_lppTimeStamp; // has to be incremented every llp time period; holds last 10 events  
  
  uint8_t Lpp10bMapToCnt (uint16_t lpp10bMap);
  void GotoNextLppTimeStamp ();
  static uint8_t CalculateNextLppTimeStamp (uint8_t currTimeStamp);
};

} // namespace aodv
} // namespace ns3


#endif /* AODVNEIGHBORETX_H */

