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
 * Authors: Mohamed Skander DAAS <daas.skander@umc.edu.dz>
 */


#include <fstream>
#include <iostream>
#include <math.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/dmsrp-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("manet-routing-compare");

void DepletionDetected (void);
class RoutingExperiment
{
public:
  RoutingExperiment ();
  void Run ();
private:
};

//--------Names parameters----------------------------------

std::string m_fileFlowMonName="FlowMonnitoTraceFile";
std::string m_fileName="manet-routing.TraceFile.csv";
std::string tr_name ("MobilityTaceFile.mob");

//--------Mobility and Psition parameters----------------------------------
int m_nodeSpeed = 19.99;		// Speed of nodes (m/s)
int m_nodePause = 0;			// pause time (seconds)
double m_segment_size=400;		// (on meters) / Define the simulation region in function of the number of nodes : one node by (m_segment_size*m_segment_size) meters^2 

//--------Nodes parameters----------------------------------

uint32_t m_mode = 3;			// SNR_AWARE_MULTI_PARENT_MODE
uint32_t m_nWifiNodes = 16;		//Number of wifi nodes.
uint32_t m_nSinks = 3;     		// Number of sinks among the number of wifi nodes.

//--------Application parameters----------------------------------

uint32_t m_port=9;
uint32_t m_packetSize = 64;		// Packet size (bytes)
double m_interval = 1;			// Interval between two transmited packets (sencends)
uint32_t m_maxPackets = 1000000;	// Maximum number of transmitted packets (100000=>unlimited)
double m_totalTime = 100.2;		// Total time of the simulation (seconds)
double m_SRVstartTime = 45;		// Start time of server application instaled in wifi nodes (seconds)
double m_CLIENTstartTime = 50;		// Start time of client application instaled in wifi nodes (seconds)
double m_SRVstopTime = m_totalTime-1;	// Stop time of server application instaled in wifi nodes (seconds)
double m_CLIENTstopTime = m_totalTime-2;// Stop time of client application instaled in wifi nodes (seconds)

//--------Transmossion parameters----------------------------------

std::string m_rate ("10000Mbps");	//
std::string m_phyMode ("DsssRate11Mbps");//
double  m_txp = 7.5;			//

//--------Energy parameters----------------------------------

double m_initEnergy = 200000;		// Initial energy of node batery, 200000=>unlimited (joules) 
double m_remainingEnergy = 0;		// The ramining energy in all nodes
bool m_depletionDetected=false;		// When the first battery deplition is detected this variable is set to true 
Time m_lifeTime = Seconds(9999999);	// Big value (unlimited)

void
DepletionDetected ()			// a function to detect the time of the first battery deplition
{
  if(!m_depletionDetected)
    {
      m_depletionDetected=true;
      m_lifeTime=Simulator::Now ();
    }
}

RoutingExperiment::RoutingExperiment ()
{
}


int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  experiment.Run ();
}

void
RoutingExperiment::Run ()
{
  Packet::EnablePrinting ();
  Ptr<UniformRandomVariable>   m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  double rand;

  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (m_phyMode));

  //Config::SetDefault  ("ns3::RangePropagationLossModel::MaxRange",DoubleValue (300)); //dms

  //Create nodes: the total number of nodes is (m_nWifiNodes+1) (The server (the first node), sink wifi nodes (the next nSinks nodes), simple wifi nodes (the rest of nodes). The number of wifi nodes is m_nWifiNodes that include sink wifi nodes and simple wifi nodes:

  NodeContainer adhocNodes;
  adhocNodes.Create (m_nWifiNodes+1);

  //Create a set of similar WifiNetDevice
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  //Create PHY objects 
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();

  //Create and manage wifi channel objects 
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  //Configure and install WifiMac objects on a collection of nodes
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue (m_phyMode), "ControlMode",StringValue (m_phyMode));
  wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));
  wifiMac.SetType ("ns3::AdhocWifiMac");

  //Install wifi devices in wifi Sink nodes and wifi Simple Nodes
  NetDeviceContainer wifiSinkDevices;
  NodeContainer wifiSinkNodes;
  NodeContainer wifiSimpleNodes;

  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    NetDeviceContainer adhocDev = wifi.Install (wifiPhy, wifiMac, adhocNodes.Get (i));
    wifiSinkDevices.Add(adhocDev);
    wifiSinkNodes.Add(adhocNodes.Get (i));
  }
  NetDeviceContainer wifiSimpleDevices; 
  for(uint32_t i=m_nSinks+1;i<m_nWifiNodes+1;i++)
  {
    NetDeviceContainer adhocDev = wifi.Install (wifiPhy, wifiMac, adhocNodes.Get (i));
    wifiSimpleDevices.Add(adhocDev);
    wifiSimpleNodes.Add(adhocNodes.Get (i));
  }

  //Define a network device container that for wifi interfaces (for communication the sinks and the simple nodes)
  NetDeviceContainer wifiDevices;
  wifiDevices.Add(wifiSinkDevices);
  wifiDevices.Add(wifiSimpleDevices);

  //Define a network device container for sink nodes and the server (for communication between the sinks and the server)
  NetDeviceContainer NeDevSinkOutContainer;
  NetDeviceContainer NeDevSrvContainer;

  //Define a wifi node container
  NodeContainer wifiNodes;
  wifiNodes.Add(wifiSinkNodes);
  wifiNodes.Add(wifiSimpleNodes);

  //Create IPv4 address helper for IPv4 address assignment
  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.0.0", "255.255.0.0");

  //Define Ipv4 Interface Container for Sink nodes, Simple Nodes and server node (each sink node have two interfaces : wifi interface for communicating with wifi nodes and another interface to communicate with the server )
  Ipv4InterfaceContainer wifiSinkInterfaces;            // to be installed on each Sink node
  Ipv4InterfaceContainer SinkInterface;                    // to be installed on each Sink node
  Ipv4InterfaceContainer wifiSimpleInterfaces;       // to be installed on each simple node
  Ipv4InterfaceContainer SrvInterface;                     // to be installed on the server node

  //Define the container of the sinks’ and the server interfaces
  Ipv4InterfaceContainer SinkInterfacesContainer;
  Ipv4InterfaceContainer SrvInterfacesContainer;

  //Define and create channels between the sinks and the server (these point to point channels are to simulate communication over the internet)
  // We create the channels first without any IP addressing information
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue (m_rate));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    NodeContainer ApSrv = NodeContainer (adhocNodes.Get (0),adhocNodes.Get (i));
    NetDeviceContainer NeDevApSrv = p2p.Install (ApSrv);
    NeDevSinkOutContainer.Add(NeDevApSrv.Get(1));
    NeDevSrvContainer.Add(NeDevApSrv.Get(0));
  }

  //Define the Mobility model and positions of nodes
  MobilityHelper mobilityAdhoc;
  ObjectFactory pos;
  std::stringstream ssPause;
  std::stringstream ssSpeed;
  Ptr<ListPositionAllocator> positionAlloc;
  Ptr<PositionAllocator> positionAlloc0 ;
  int64_t streamIndex = 0;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+std::to_string((std::sqrt(m_nWifiNodes))*m_segment_size)+"]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+std::to_string((std::sqrt(m_nWifiNodes))*m_segment_size)+"]"));
  positionAlloc0 = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += positionAlloc0->AssignStreams (streamIndex);
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_nodeSpeed << "]";
  ssPause << "ns3::ConstantRandomVariable[Constant=" << m_nodePause << "]";
  mobilityAdhoc.SetPositionAllocator (positionAlloc0);
  mobilityAdhoc.SetPositionAllocator (positionAlloc0);
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel","Speed", StringValue (ssSpeed.str ()),                                  "Pause", StringValue (ssPause.str ()),"PositionAllocator", PointerValue (positionAlloc0));
  mobilityAdhoc.Install(adhocNodes);
  //Create energy sources and install it on wifi devices (Sink nodes and wifi Simple Nodes)
  BasicEnergySourceHelper basicSourceHelper;
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (m_initEnergy));
  EnergySourceContainer sources = basicSourceHelper.Install (wifiNodes);

  //Define DMSRP routing protocol
  DmsrpHelper dmsrp;
  Ipv4ListRoutingHelper list;
  list.Add (dmsrp, 100);

  //Define the internet stack
  InternetStackHelper internet;
  internet.SetRoutingHelper (list);

  //Configure nodes (their type, their functioning mode etc) and install the internet stack
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::Routingmode", UintegerValue (m_mode));
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::SinkGateWayAddress", Ipv4AddressValue ("10.1.0.1"));                 // The Ip address is temporary, it will be assigned later
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::SinkOutPutDevice", PointerValue (NeDevSinkOutContainer.Get(0)));
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::ServerAddress", Ipv4AddressValue ("10.1.0.1"));                      // The Ip address is temporary, it will be assigned later

  //Define the server node and install the internet stack
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsServer",BooleanValue (true));       
  internet.Install (adhocNodes.Get (0));
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsServer",BooleanValue (false));

  // Define IPV4 server interfaces
  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    SrvInterface = addressAdhoc.Assign (NeDevSrvContainer.Get(i-1));
    SrvInterfacesContainer.Add(SrvInterface);  // memorize server interfaces for printing to use them later as gateways for sinks
  }

  //Define the simple nodes and install the internet stack
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
  for(uint32_t i=m_nSinks+1;i<m_nWifiNodes+1;i++)
  {
    internet.Install (adhocNodes.Get (i));
  }

  //Assign IP addresses for simple nodes
  wifiSimpleInterfaces = addressAdhoc.Assign (wifiSimpleDevices);

  //Define the sink nodes and install the internet stack
  Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (true));
  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    Config::SetDefault("ns3::dmsrp::RoutingProtocol::SinkGateWayAddress",Ipv4AddressValue( SrvInterfacesContainer.GetAddress (i-1))); 
    Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::SinkOutPutDevice", PointerValue (NeDevSinkOutContainer.Get(i-1)));
    Config::SetDefault("ns3::dmsrp::RoutingProtocol::ServerAddress",Ipv4AddressValue( SrvInterfacesContainer.GetAddress(i-1)));
    internet.Install (adhocNodes.Get (i));
  }

  //Assign IP addresses for sink nodes
  //Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
  wifiSinkInterfaces = addressAdhoc.Assign (wifiSinkDevices);
  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    SinkInterface = addressAdhoc.Assign (NeDevSinkOutContainer.Get(i-1));
    SinkInterfacesContainer.Add(SinkInterface);  // memorize server interfaces for printing
  }

  //Print an overview of the network architecture
  for(uint32_t i=1;i<=m_nSinks;i++)
  {
    NS_LOG_UNCOND("Sink wifi "<<i<< ": "<<(wifiSinkDevices.Get(i-1))->GetAddress()<<": IPv4: "<<  wifiSinkInterfaces.GetAddress(i-1));//SinkInterfacesContainer
  }
  for(uint32_t i=0;i<  m_nSinks;i++)
  {
    NS_LOG_UNCOND("Server out"<<i<< ": "<<(NeDevSrvContainer.Get(i))->GetAddress()<<": IPv4: "<<  SrvInterfacesContainer.GetAddress(i));
  }
  for(uint32_t i=m_nSinks+1;i<m_nWifiNodes+1;i++)
  {
    NS_LOG_UNCOND("Node "<<i<< ": "<<(wifiSimpleDevices.Get(i-1-m_nSinks))->GetAddress()<<": IPv4: "<<  wifiSimpleInterfaces.GetAddress(i-1-m_nSinks));
  }

  uint32_t rand_int;
  ApplicationContainer apps;
  UdpServerHelper server (m_port);

  //Install and run the server application on all nodes
  for(uint32_t i=0;i<m_nWifiNodes+1;i++)
  {
    ApplicationContainer apps = server.Install (adhocNodes.Get (i)); 
    apps.Start (Seconds (m_SRVstartTime));
    apps.Stop (Seconds (m_SRVstopTime));
  }

  //Install and run the client application on simple and sink nodes (Each node chooses randomly its destination among the other nodes to send it data packets)
  for(uint32_t i=0;i<m_nWifiNodes;i++)
  {
  rand=m_uniformRandomVariable->GetValue(0,m_nWifiNodes);
  rand_int=floor(rand);
  Address serverAddress;
    if(rand_int<m_nSinks)
    {
      serverAddress =Address (Ipv4Address(wifiSinkInterfaces.GetAddress (rand_int)));// Define a server application ip address
    }
    else
    {
      serverAddress =Address (Ipv4Address(wifiSimpleInterfaces.GetAddress (rand_int-m_nSinks)));                  
    }

    UdpClientHelper client (serverAddress, m_port);
    client.SetAttribute ("MaxPackets", UintegerValue (m_maxPackets));
    client.SetAttribute ("Interval", TimeValue (Seconds (m_interval)));
    client.SetAttribute ("PacketSize", UintegerValue (m_packetSize));

    apps = client.Install (adhocNodes.Get (i+1));
    rand=m_uniformRandomVariable->GetValue(0,5);
    apps.Start (Seconds (m_CLIENTstartTime+rand));
    apps.Stop (Seconds (m_CLIENTstopTime+rand));
  }

  //Enable ascii trace associated with the devices. 
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name));

  //Install a Flow Monitor module to provide a flexible system to measure the performance of network protocols.
  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

  //Generate Animation trace file
  AnimationInterface anim("AnimTraceFile.xml");
  anim.SetMaxPktsPerTraceFile(9000000);


  // When a source battery depletion is detected the function DepletionDetected is called (can be used to calculate the network lifetime)
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::dmsrp::RoutingProtocol/DepTime", MakeCallback(&DepletionDetected));

  // Define the simulation time and run it
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();

  // Calculate the remaining Energy on the network nodes
  Ptr<EnergySource> EnergySrc;
  for(uint32_t i=0;i< sources.GetN();i++)
  {
    EnergySrc = sources.Get (i);
    m_remainingEnergy=m_remainingEnergy+EnergySrc->GetRemainingEnergy ();
  }

  NS_LOG_UNCOND("Total remainig energy is: "<<m_remainingEnergy);
  NS_LOG_UNCOND("Network life time is: "<<m_lifeTime);
  //Output of the trace file of the flowmonitor 
  flowmon->SerializeToXmlFile ((m_fileFlowMonName + ".xml").c_str(), false, false);

  Simulator::Destroy ();
}





























