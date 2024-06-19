#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ping-helper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace ns3;

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

int main(int argc, char **argv)
{
  uint32_t size=50;
  double step=100;
  double totalTime=100;

  int packetSize = 1024;
  int totalPackets = totalTime-1;
  double interval = 1.0; 
  Time interPacketInterval = Seconds (interval);
  
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);
  
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(1),
                                  "DeltaY",
                                  DoubleValue(1),
                                  "GridWidth",
                                    UintegerValue(10),
                                  "LayoutType",
                                  StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);
 
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
  devices = wifi.Install(wifiPhy, wifiMac, nodes);

  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); 
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (size-1), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (0), tid);
  InetSocketAddress remote = InetSocketAddress (interfaces.GetAddress (size-1,0), 8080);
  source->Connect (remote);
  
  PingHelper ping(interfaces.GetAddress(size - 1));
  ping.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));

  ApplicationContainer p = ping.Install(nodes.Get(0));
  p.Start(Seconds(0));
  p.Stop(Seconds(totalTime) - Seconds(0.001));

  // move node away
  Ptr<Node> mov_node = nodes.Get(size / 2);
  Ptr<MobilityModel> mob = mov_node->GetObject<MobilityModel>();
  Simulator::Schedule(Seconds(66),
                      &MobilityModel::SetPosition,
                      mob,
                      Vector(100, 100, 0));

  Simulator::Schedule (Seconds (1), &GenerateTraffic, source, packetSize, totalPackets, interPacketInterval);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
                       
  std::cout << "Starting simulation for " << totalTime << " s ...\n";
  AnimationInterface anim ("scratch/aodv-output.xml");
  
  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();
  
	      
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/aodv-flow.xml", true, true);
  Simulator::Destroy ();
  
  std::cout<<"\n\n***** OUTPUT *****\n\n";
  std::cout<<"Total Packets sent = "<<packetsSent<<std::endl;
  std::cout<<"Total Packets received = "<<packetsReceived<<std::endl;
  float packetsRatio = ((float) packetsReceived/packetsSent)*100;
  std::cout<<"Packet delivery ratio = "<<packetsRatio<<" %"<<std::endl;
  
}
