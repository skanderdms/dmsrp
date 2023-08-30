# dmsrp
An ns3 module of a hierarchical routing protocol for distributed mobile wireless networks
.. include:: replace.txt

Dynamic Multi Sink Routing Protocol (DMS-RP)
--------------------------------------------

This model implements the base specification of the Dynamic Multi Sink 
Routing Protocol (DMS-RP) protocol.

The model was written by Mohamed Skander DAAS, University of Freres Mentouri Constantine 1.

Model Description
-----------------

The source code for the DMS-RP model lives in the directory `src/dmsrp`.

Design
------

Class ``ns3::dmsrp::RoutingProtocol`` implements all functionality of 
service packet exchange and inherits from ``ns3::Ipv4RoutingProtocol``.
The base class defines two virtual functions for packet routing and 
forwarding.  The first one, ``ns3::dmsrp::RouteOutput``, is used for 
locally originated packets, and the second one, ``ns3::dmsrp::RouteInput``,
is used for forwarding and/or delivering received packets.

Protocol operation depends on many adjustable parameters. Parameters for 
this functionality are attributes of ``ns3::dmsrp::RoutingProtocol``. 
Parameter default values allow the enabling/disabling protocol features, such as routing mode functionning and so on.

How to install
--------------

- Install the ns3 allinone simulator (https://www.nsnam.org/).

- Copy the dmsrp folder to the source folder 'src' where ns3 is intalled:

    cp /pathdmsrp/dmsrp/  /pathns/ns-x.xx/scr/

  Where x-xx is the ns3 version.

- Set the current path to the ns3 root and rebuild the ns3 with the dmsrp module:

    cd /pathns/ns-x.xx/

    ./waf

How to simulate a network using DMSRP protocol
----------------------------------------------
Creating a network simulation in ns3 using DMSRP routing protocol involves several steps. Here's a simplified steps to get started. Note that these steps only cover the basics.

- Setting up the ns3 environment: Make sure that ns3 is installed. You can follow the installation instructions available on the official website of ns3: https://www.nsnam.org/.

- Creating of a simulation script: Create a script in C++ to describe the simulation of the network. An example of a detailled and commented script is given in (https://github.com/skanderdms/dmsrp/blob/main/dmsrp/examples/example.cc). This script could be customized for a personalized simulation.

- Configuring the simulation components: This step needs to configure nodes, devices, channels, mobility models, protocols and applications to a specific need to simulate a network.

- Running the simulation: Once the simulation is configured, the C++ script ("example.cc") file should be put in the Scratch ns3 forder (default), set the current path to the ns3 root and run the simulation script using the waf command:

		cd /pathns/ns-x.xx/

		./waf -run example

Documentation on how to simulate a network using DMSRP protocol
---------------------------------------------------------------

The following lines show how DMSRP is used in a C++ simulation script in ns3 :

Define the DMSRP routing protocol
--------------------------------- 

	DmsrpHelper dmsrp;
  
Configure the routing mode
-------------------------- 

	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::Routingmode", UintegerValue (m_mode));

Define the routing for a server node
------------------------------------ 

	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
	
	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsServer",BooleanValue (true)); 

Define the routing for a sink node
---------------------------------- 

	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
	
	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsServer",BooleanValue (true));
  
Define the routing for a simple node
------------------------------------ 

	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsSink", BooleanValue (false));
	
	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::IsServer",BooleanValue (false));

  
Define a sink gateway address (Point a server interface address)
---------------------------------------------------------------- 

	Config::SetDefault("ns3::dmsrp::RoutingProtocol::SinkGateWayAddress",Ipv4AddressValue(xxxxx)); 
	
Define a sink output device
--------------------------- 

	Config::SetDefault  ("ns3::dmsrp::RoutingProtocol::SinkOutPutDevice", PointerValue (xxxxx));
	

Define a server interface address	
--------------------------------- 

	Config::SetDefault("ns3::dmsrp::RoutingProtocol::ServerAddress",Ipv4AddressValue(xxxxx));
	
	
When a source battery depletion is detected the function DepletionDetected is called
------------------------------------------------------------------------------------


	Config::ConnectWithoutContext ("/NodeList/*/$ns3::dmsrp::RoutingProtocol/DepTime", MakeCallback(&DepletionDetected));
  

Define the period of time during which the route is considered to be valid
-------------------------------------------------------------------------- 
  
	Config::SetDefault("ns3::dmsrp::RoutingProtocol::ActiveRouteTimeout",Seconds(xxxxx));
	
