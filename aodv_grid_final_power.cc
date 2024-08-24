#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace ns3;

// Global variables tracking package ratio
int packetsSent = 0;
int packetsReceived = 0;

// Prints out data on the received packet from a sink socket.
// Formatted as a function to be used as a variable.
// Input: Socket pointer
// Return: None
void ReceivePacket (Ptr<Socket> socket) {
  Ptr<Packet> packet;

  // As the socket receives a packet, it prints its parameters and adds to the count.
  while ((packet = socket->Recv ())) {
	  packetsReceived++;
      std::cout<<"Received packet - "<<packetsReceived<<" and Size is "<<packet->GetSize ()<<" Bytes."<<std::endl;
  }
}

// Repeatedly sends a packet from source to sink using recursion. 
// Formatted as a function to be used as a recursive variable.
// Input: Socket pointer, packet size, packet count, packet sending interval.
// Return: None
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, 
                             uint32_t pktCount, Time pktInterval ) {
  //Only send packets when the count is still positive, else close the socket
  if (pktCount > 0) {
      // Creates a packet and sends it through the socket
      socket->Send (Create<Packet> (pktSize));
      packetsSent++;
      std::cout<<"Packet sent - "<<packetsSent<<std::endl;
      
      // After a time interval, re-send the packet with a recursive call of the function
      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval); 
  }
  else {
      socket->Close ();
  }
}

int main(int argc, char **argv)
{
// Initial variables related to the topology. 
  uint32_t size = 50;
  double totalTime = 60;
  int rowLength = 10;
  int numMove = 12;
  double step = 136;
  long int maxPower = 1e5;
 
  int packetSize = 1024;
  int totalPackets = 5;
  double interval = 1.0; 
  Time interPacketInterval = Seconds (interval);
 
// Initializes the node, interface, and device container. 
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);

// Creates the 5x10 grid topology
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",
                                DoubleValue(0.0),
                                "MinY",
                                DoubleValue(0.0),
                                "DeltaX",
                                DoubleValue(step),
                                "DeltaY",
                                DoubleValue(step),
                                "GridWidth",
                                UintegerValue(rowLength),
                                "LayoutType",
                                StringValue("RowFirst"));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(nodes);
 
// Initializes the wifi mac address settings
  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  
// Sets the power level at the start and end
  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.Set("TxPowerStart", DoubleValue(maxPower));
  wifiPhy.Set("TxPowerEnd", DoubleValue(maxPower));
 
// Sets up the wifi devices
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("OfdmRate54Mbps"),
                                 "RtsCtsThreshold",
                                 UintegerValue(0));
  devices = wifi.Install(wifiPhy, wifiMac, nodes);
  wifiPhy.EnablePcapAll(std::string("PCAP/aodv_grid_final_power/aodv"));

  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); 
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
  
  Ptr<OutputStreamWrapper> routingStream =
  Create<OutputStreamWrapper>("PCAP/aodv_grid_final_power/aodv.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(totalTime), routingStream);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (size-1), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (0), tid);
  InetSocketAddress remote = InetSocketAddress (interfaces.GetAddress (size-1,0), 8080);
  source->Connect (remote);
  
  Simulator::Schedule (Seconds (0), &GenerateTraffic, source, packetSize, totalPackets, interPacketInterval);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  for(int i = 1; i <= numMove; i ++) {

    Ptr<Node> node1 = nodes.Get(1+4*(i-1));
    Ptr<Node> node2 = nodes.Get(2+4*(i-1));
    Ptr<Node> node3 = nodes.Get(3+4*(i-1));
    Ptr<MobilityModel> mob1 = node1->GetObject<MobilityModel>();
    Ptr<MobilityModel> mob2 = node2->GetObject<MobilityModel>();
    Ptr<MobilityModel> mob3 = node3->GetObject<MobilityModel>();

    Simulator::Schedule(Seconds(i),
                        &MobilityModel::SetPosition,
                        mob1,
                        Vector(double(2.0*rowLength*step), double(2.0*rowLength*step), 0));
    Simulator::Schedule(Seconds(i),
                        &MobilityModel::SetPosition,
                        mob2,
                        Vector(double(2.0*rowLength*step), double(2.0*rowLength*step), 0));
    Simulator::Schedule(Seconds(i),
                        &MobilityModel::SetPosition,
                        mob3,
                        Vector(double(2.0*rowLength*step), double(2.0*rowLength*step), 0));

  }
  
                   
  std::cout << "Starting simulation for " << totalTime << " s ...\n";
  AnimationInterface anim ("Simulation/aodv_grid_final_power/3_nodes/grid_aodv_output.xml");
  anim.SetMaxPktsPerTraceFile(1000000);
  anim.EnablePacketMetadata ();
  
  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();
  
  Simulator::Stop (Seconds (totalTime)); 
  Simulator::Run ();
  flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.01));
  flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(1));
  flowmon->CheckForLostPackets();
  flowmon->SerializeToXmlFile("Simulation/aodv_grid_final_power/3_nodes/grid_aodv_flow.xml", true, true);
  Simulator::Destroy ();
  
  std::cout<<"\n\n***** OUTPUT *****\n\n";
  std::cout<<"Total Packets sent = "<<packetsSent<<std::endl;
  std::cout<<"Total Packets received = "<<packetsReceived<<std::endl;
  float packetsRatio = ((float) packetsReceived/packetsSent)*100;
  std::cout<<"Packet delivery ratio = "<<packetsRatio<<" %"<<std::endl; 
}
