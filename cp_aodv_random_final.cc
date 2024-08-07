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
#include "ns3/flow-monitor-module.h"
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

int main (int argc, char *argv[])
{
  // Create nodes
  uint32_t size=6;
  double totalTime=10;
  int totalPackets = totalTime-1;

  int packetSize = 1024;
  double interval = 1.0;
  Time interPacketInterval = Seconds (interval);

  NodeContainer groundNodes;
  groundNodes.Create(size);  // mobile ground nodes

  NodeContainer uavNode;
  uavNode.Create(1);  // 1 UAV node (server)

  // Install mobility models
  MobilityHelper mobility;
 
  // Set initial positions for ground nodes
  for (uint32_t i = 0; i < size; i++) {
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(i * 20.0, 0.0, 0.0)); // Initial positions with a separation of 20 meters
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(groundNodes.Get(i));
  }

  // Set initial position for UAV
  Ptr<ListPositionAllocator> uavPositionAlloc = CreateObject<ListPositionAllocator>();
  uavPositionAlloc->Add(Vector(10.0, 20.0*(size-1), 0.0)); // Initial position for UAV
  mobility.SetPositionAllocator(uavPositionAlloc);
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(uavNode);

  // WifiMac
  WifiMacHelper staWifiMac;
  WifiMacHelper apWifiMac;
  staWifiMac.SetType("ns3::AdhocWifiMac");
  apWifiMac.SetType("ns3::ApWifiMac");

  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiPhy.SetChannel(wifiChannel.Create());
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("OfdmRate6Mbps"),
                                 "RtsCtsThreshold",
                                 UintegerValue(0));

  NetDeviceContainer staDevice = wifi.Install(wifiPhy, staWifiMac, groundNodes); 
  NetDeviceContainer apDevice = wifi.Install(wifiPhy, apWifiMac, uavNode);
  
  wifiPhy.EnablePcapAll(std::string("PCAP/aodv_random_final/aodv"));

  // Set up AODV
  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (groundNodes);
  stack.Install (uavNode);

  // Set up Ipv4 Address
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer groundInterfaces = address.Assign (staDevice);
  Ipv4InterfaceContainer uavInterfaces = address.Assign (apDevice);

  // Enable PCAP
  Ptr<OutputStreamWrapper> routingStream =
  Create<OutputStreamWrapper>("PCAP/aodv_random_final/aodv.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(totalTime), routingStream);

  // UAV connect with ground
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (groundNodes.Get (size/2), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (uavNode.Get(0), tid);
  InetSocketAddress remote = InetSocketAddress (groundInterfaces.GetAddress (size/2,0), 8080);
  source->Connect (remote);

  // Simulator
  Simulator::Schedule (Seconds (1), &GenerateTraffic, source, packetSize, totalPackets, interPacketInterval);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";
  AnimationInterface anim ("scratch/random_aodv_output.xml");

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();


  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("scratch/random_aodv_flow.xml", true, true);
  Simulator::Destroy ();

  std::cout<<"\n\n***** OUTPUT *****\n\n";
  std::cout<<"Total Packets sent = "<<packetsSent<<std::endl;
  std::cout<<"Total Packets received = "<<packetsReceived<<std::endl;
  float packetsRatio = ((float) packetsReceived/packetsSent)*100;
  std::cout<<"Packet delivery ratio = "<<packetsRatio<<" %"<<std::endl;

  return 0;
} 
