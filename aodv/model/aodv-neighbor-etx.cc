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
 
#include "aodv-neighbor-etx.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <math.h>
#include <stdint.h>

namespace ns3
{
  
NS_LOG_COMPONENT_DEFINE ("AodvNeighborEtx");

namespace aodv
{

NeighborEtx::NeighborEtx () : m_lppTimeStamp (0) {}

// ETX timestamp has 12 different values: 0, 1, 2, ..., 11, 0, 1, ...
// but 2 values are not included in etx (lpp count): current and oldest

uint8_t
NeighborEtx::CalculateNextLppTimeStamp (uint8_t currTimeStamp)
{
  uint8_t nextTimeStamp = currTimeStamp + 1;
  if (nextTimeStamp > 11)
    {
      nextTimeStamp = 0;
    }
  return nextTimeStamp;
}

void
NeighborEtx::GotoNextLppTimeStamp ()
{
  m_lppTimeStamp = CalculateNextLppTimeStamp (m_lppTimeStamp);
}


// ETX timestamp has 12 different values: 0, 1, 2, ..., 11, 0, 1, ...
// but 2 values are not included in etx (lpp count):
// 1. curent time stamp because of jitter some nodes where transmited before this node 
//    and some nodes will transmit after so packet count would not be fair (nodes transmitted before
//    would be having one more higher lpp count)
// 2. oldest time stamp will be deleted so it sholud not be ncluded in calculation of lpp count
uint8_t 
NeighborEtx::Lpp10bMapToCnt (uint16_t lpp10bMap)
{
  uint8_t lpp = 0;
  for (int j=0; j<12; ++j)
    {
      if ((j!=m_lppTimeStamp) && (j!=(m_lppTimeStamp+1)))
        {
          lpp += (lpp10bMap & ((uint16_t)0x0001 << j)) ? 1 : 0;
        }
    }
  return lpp;
}

void
NeighborEtx::GotoNextTimeStampAndClearOldest ()
{
  GotoNextLppTimeStamp (); // go to  next timestamp which becomes current timestamp
  // Clear oldest timestamp lpp count values, this is next time stamp compared to current
  //NS_LOG_UNCOND ("ETX: " << Simulator::Now ().GetSeconds () << ", " << (uint16_t)m_lppTimeStamp << ", " << (uint16_t)(~((uint16_t)0x0001 << CalculateNextLppTimeStamp (m_lppTimeStamp))));
  for (std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
    {
      Etx etx = i->second;
      uint16_t lppMyCnt10bMap = etx.m_lppMyCnt10bMap;
      // Delete oldest timestamp lpp count (this is next time stamp compared to current)
      // Only lower 12 bits are used
      lppMyCnt10bMap &= (uint16_t)(~((uint16_t)0x0001 << CalculateNextLppTimeStamp (m_lppTimeStamp)) & (uint16_t)0x0FFF);
      etx.m_lppMyCnt10bMap = lppMyCnt10bMap;
      i->second = etx;
    }  
}

void
NeighborEtx::FillLppCntData (LppHeader &lppHeader)
{
  for (std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
        {
          uint8_t lpp = Lpp10bMapToCnt (i->second.m_lppMyCnt10bMap);
          if (lpp > 0)
            {
              lppHeader.AddToNeighborsList (i->first, lpp);
              //NS_LOG_UNCOND ("           IP=" << i->first << ", lpp=" << (uint16_t)lpp << ", rev=" << (uint16_t)(i->second.m_lppReverse) << ", ETX-bin-shift=" << CalculateBinaryShiftedEtx (i->second));
            }
        }
}

bool 
NeighborEtx::UpdateNeighborEtx (Ipv4Address addr, uint8_t lppTimeStamp, uint8_t lppReverse)
{
  std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, insert new entry
      Etx etx;
      etx.m_lppReverse = lppReverse;
      etx.m_lppMyCnt10bMap = (uint16_t)0x0001 << lppTimeStamp;
      std::pair<std::map<Ipv4Address, Etx>::iterator, bool> result = m_neighborEtx.insert (std::make_pair (addr, etx));
      return result.second;
    }
  else
    {
      // Address found, update existing entry
      i->second.m_lppReverse = lppReverse;
      (i->second.m_lppMyCnt10bMap) |= ((uint16_t)0x0001 << lppTimeStamp);
      return true;
    }
}

uint32_t 
NeighborEtx::CalculateBinaryShiftedEtx (Etx etxStruct)
{
  uint32_t etx = UINT32_MAX;
  if ((Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap)!=0) && (etxStruct.m_lppReverse!=0))
    {
      etx = (uint32_t) (round (1000000.0 / (Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap) * etxStruct.m_lppReverse)));
    }
  //NS_LOG_UNCOND ("ETX binary: " << etx);
  return etx;
}

uint32_t 
NeighborEtx::ReadEtxForNeighbor (Ipv4Address addr)
{
  uint32_t etx;
  std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, ETX -> oo (= UINT32_MAX)
      etx = UINT32_MAX;
      return etx;
    }
  else
    {
      // Address found, calculate and return current ETX value
      return CalculateBinaryShiftedEtx (i->second);
    }
}



} // namespace aodv
} // namespace ns3
