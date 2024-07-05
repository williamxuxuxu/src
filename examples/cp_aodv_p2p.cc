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
#include "ns3/aodv-module.h"
#include "ns3/ping-helper.h"
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE("CustomExample");

int packetsSent = 0;
int packetsReceived = 0;

void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
      packetsReceived++;
      std::cout<<"Received packet - "<<packetsReceived<<" and Size is "<<packet->GetSize ()<<" Bytes."<<std::endl;
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      packetsSent++;
      std::cout<<"Packet sent - "<<packetsSent<<std::endl;

      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int
main(int argc, char* argv[])
{

    // step-1 = creating group of nodes....
    NodeContainer allNodes, nodes01, nodes12, nodes23, nodes34, nodes45, nodes56, nodes15;
    allNodes.Create(7);
     
    nodes01.Add(allNodes.Get(0));
    nodes01.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(1));
    nodes12.Add(allNodes.Get(2));
    nodes23.Add(allNodes.Get(2));
    nodes23.Add(allNodes.Get(3));
    nodes34.Add(allNodes.Get(3));
    nodes34.Add(allNodes.Get(4));
    nodes45.Add(allNodes.Get(4));
    nodes45.Add(allNodes.Get(5));
    nodes56.Add(allNodes.Get(5));
    nodes56.Add(allNodes.Get(6));
    nodes15.Add(allNodes.Get(1));
    nodes15.Add(allNodes.Get(5));

    // step-2 = create link
    PointToPointHelper p2pl1;
    p2pl1.SetDeviceAttribute("DataRate",StringValue("200MB/s"));
    p2pl1.SetChannelAttribute("Delay",StringValue("1ms"));
     
     // step-3 = creating devices
    NetDeviceContainer  devices01, devices12, devices23, devices34, devices45, devices56, devices15;
    devices01 = p2pl1.Install(nodes01);
    devices12 = p2pl1.Install(nodes12);
    devices23 = p2pl1.Install(nodes23);
    devices34 = p2pl1.Install(nodes34);
    devices45 = p2pl1.Install(nodes45);
    devices56 = p2pl1.Install(nodes56);
    devices15 = p2pl1.Install(nodes15);

     
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
    Ipv4InterfaceContainer interfaces34 = address.Assign(devices34);
     
     
    address.SetBase("58.0.0.0","255.0.0.0");
    Ipv4InterfaceContainer interfaces45 = address.Assign(devices45);
     
     
    address.SetBase("59.0.0.0","255.0.0.0");
    Ipv4InterfaceContainer interfaces56 = address.Assign(devices56);
     
    
    address.SetBase("60.0.0.0","255.0.0.0");
    Ipv4InterfaceContainer interfaces15 = address.Assign(devices15);

 
    // step-6 = server configuration
    UdpEchoServerHelper echoServer(54);
     
    ApplicationContainer serverApps = echoServer.Install(allNodes.Get(6));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(11.0));
     
    // step-7 = client configuration
    UdpEchoClientHelper echoClient(interfaces56.GetAddress(1),54);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
     
    ApplicationContainer clientApps = echoClient.Install(allNodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(11.0));

/* 
    // Step-2 = create devices 
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("OfdmRate6Mbps"),
                                 "RtsCtsThreshold",
                                 UintegerValue(0));

    devices01 = wifi.Install(wifiPhy, wifiMac, nodes01);
    devices12 = wifi.Install(wifiPhy, wifiMac, nodes12);
    devices23 = wifi.Install(wifiPhy, wifiMac, nodes23);
    devices34 = wifi.Install(wifiPhy, wifiMac, nodes34);
    devices45 = wifi.Install(wifiPhy, wifiMac, nodes45);
    devices56 = wifi.Install(wifiPhy, wifiMac, nodes56);
    devices15 = wifi.Install(wifiPhy, wifiMac, nodes15);
    
    
    // Step-3 = Configure AODV
    AodvHelper aodv;
    // you can configure AODV attributes here using aodv.Set(name, value)
    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv); // has effect on the next Install ()
    stack.Install(allNodes);
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interface01, interface12, interface23, interface34, interface45, interface56, interface15;

    interface01 = address.Assign(devices01);
    interface12 = address.Assign(devices12);
    interface23 = address.Assign(devices23);
    interface34 = address.Assign(devices34);
    interface45 = address.Assign(devices45);
    interface56 = address.Assign(devices56);
    interface15 = address.Assign(devices15);

    // Step-4 = Ping
    PingHelper ping(interface56.GetAddress(1));
    ping.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));

    ApplicationContainer p = ping.Install(allNodes.Get(0));
    p.Start(Seconds(0));
    p.Stop(Seconds(5));
 */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    AnimationInterface anim("animationCustom.xml");
     
    anim.SetConstantPosition(allNodes.Get(0), 10.0, 10.0);
    anim.SetConstantPosition(allNodes.Get(1), 20.0, 10.0);
    anim.SetConstantPosition(allNodes.Get(2), 20.0, 20.0);
    anim.SetConstantPosition(allNodes.Get(3), 30.0, 20.0);
    anim.SetConstantPosition(allNodes.Get(4), 40.0, 20.0);
    anim.SetConstantPosition(allNodes.Get(5), 40.0, 10.0);
    anim.SetConstantPosition(allNodes.Get(6), 50.0, 10.0);

 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
