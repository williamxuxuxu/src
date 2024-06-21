#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-standards.h"
#include "ns3/netanim-module.h"
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE("CustomExample");
 
int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);
 
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
 
    // step-1 = creating group of nodes....
    NodeContainer allNodes,nodes01,nodes12,nodes23,nodes03,nodes02,nodes13;
allNodes.Create(4);
 
nodes01.Add(allNodes.Get(0));
nodes01.Add(allNodes.Get(1));
nodes12.Add(allNodes.Get(1));
nodes12.Add(allNodes.Get(2));
nodes23.Add(allNodes.Get(2));
nodes23.Add(allNodes.Get(3));
nodes03.Add(allNodes.Get(0));
nodes03.Add(allNodes.Get(3));
nodes02.Add(allNodes.Get(0));
nodes02.Add(allNodes.Get(2));
nodes13.Add(allNodes.Get(1));
nodes13.Add(allNodes.Get(3));
 
// step-2 = create link
PointToPointHelper p2pl1;
p2pl1.SetDeviceAttribute("DataRate",StringValue("200MB/s"));
p2pl1.SetChannelAttribute("Delay",StringValue("1ms"));
 
 // step-3 = creating devices
NetDeviceContainer  devices01, devices12, devices23, devices03, devices02, devices13;
devices01 = p2pl1.Install(nodes01);
devices12 = p2pl1.Install(nodes12);
devices23 = p2pl1.Install(nodes23);
devices03 = p2pl1.Install(nodes03);
devices02 = p2pl1.Install(nodes02);
devices13 = p2pl1.Install(nodes13);
MobilityHelper mobility;
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.Install(allNodes);
 
// step-4 = Install ip stack
InternetStackHelper stack;
stack.Install(allNodes);
 
// step-5 = Assignment of IP Address
Ipv4AddressHelper address;
 
 
address.SetBase("54.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
 
 
address.SetBase("55.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
 
 
address.SetBase("56.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces23 = address.Assign(devices23);
 
 
address.SetBase("57.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces03 = address.Assign(devices03);
 
 
address.SetBase("58.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);
 
 
address.SetBase("59.0.0.0","255.0.0.0");
Ipv4InterfaceContainer interfaces13 = address.Assign(devices13);
 
 
// step-6 = server configuration
UdpEchoServerHelper echoServer(54);
 
ApplicationContainer serverApps = echoServer.Install(allNodes.Get(2));
serverApps.Start(Seconds(1.0));
serverApps.Stop(Seconds(11.0));
 
// step-7 = client configuration
UdpEchoClientHelper echoClient(interfaces12.GetAddress(1),54);
echoClient.SetAttribute("MaxPackets", UintegerValue(5));
echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
echoClient.SetAttribute("PacketSize", UintegerValue(1024));
 
ApplicationContainer clientApps = echoClient.Install(allNodes.Get(0));
clientApps.Start(Seconds(1.0));
clientApps.Stop(Seconds(11.0));
 
AnimationInterface anim("animationCustom.xml");
Ipv4GlobalRoutingHelper::PopulateRoutingTables();
 
 
 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
