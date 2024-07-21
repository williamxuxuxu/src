#include "ns3/core-module.h"
#include "ns3/aodv-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"

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

int main (int argc, char* argv[]) 
{
  bool enableFlowMonitor = true;
  CommandLine cmd;
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);
  uint32_t jumlah_node = 10; // number of nodes
  double waktu_simulasi = 200; // simulation time
  std::string phyMode ("DsssRate1Mbps");

  Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("512")); // packet size
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("1000kb/s")); // data rate

  // devices setting
  //  nodes
  NodeContainer node;
  NodeContainer node_manet;
  node.Create(jumlah_node);

  //  wifi
  WifiHelper wifi;
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChan = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel(wifiChan.Create());

  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");

  wifi.SetStandard (WIFI_STANDARD_80211b);

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue(phyMode), "ControlMode",StringValue(phyMode));

  NetDeviceContainer device;
  device = wifi.Install(wifiPhy,mac,node);

  // set AODV routing protocol
  AodvHelper aodv;
  AodvHelper malicious_aodv; 
  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv);
  internet.Install (node_manet);
  internet.Install(node);
 
  // set IP to all devices
  Ipv4AddressHelper ipadd;
  ipadd.SetBase("192.168.0.0", "255.255.255.0");
  ipadd.Assign(device);

  // set mobility
  MobilityHelper mobilitas;
  ObjectFactory pos;
  pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
  Ptr<PositionAllocator> posAlloc = pos.Create()->GetObject<PositionAllocator>();
  mobilitas.SetMobilityModel("ns3::RandomWaypointMobilityModel", 
                            "Speed", StringValue ("ns3::UniformRandomVariable[Min=0|Max=60]"),
                            "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"),
                            "PositionAllocator", PointerValue(posAlloc));
  mobilitas.SetPositionAllocator (posAlloc);
  mobilitas.Install(node);
  
 // set UDP connection
  uint32_t port = 5000;
  OnOffHelper cbrgenhelper("ns3::UdpSocketFactory", Address());
  
  cbrgenhelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=42.0]"));
  cbrgenhelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

  ApplicationContainer cbrgen;
  AddressValue remoteaddr;

  for (uint32_t j = 0; j < 4; j++)
  {
    AddressValue remoteadd(InetSocketAddress(ipadd.Assign(device).GetAddress(j+1),port));
    cbrgenhelper.SetAttribute("Remote", remoteadd);
    cbrgen.Add(cbrgenhelper.Install(node.Get(j)));
  }

  cbrgen.Start(Seconds(0.0));
  cbrgen.Stop(Seconds(waktu_simulasi-10.0));

 // set packet sink
  for (uint32_t k = 0; k < 4; k++)
  {
    PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress(ipadd.Assign(device).GetAddress(k+1),port));
    cbrgen.Add(sink.Install(node.Get(k+1)));
  }

  cbrgen.Start(Seconds(0.0));
  cbrgen.Stop(Seconds(waktu_simulasi-10.0));

  
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(waktu_simulasi));
  wifiPhy.EnablePcap("test", device.Get(0),true);
  AnimationInterface anim ("manet.xml");
  Simulator::Run();
  
  monitor->CheckForLostPackets ();

  // calculate tx, rx, throughput, end to end delay, dand packet delivery ratio
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if ((t.sourceAddress=="192.168.0.2" && t.destinationAddress == "192.168.0.4"))
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
          std::cout << "  End to End Delay: " << i->second.delaySum.GetSeconds() / i->second.txPackets <<"\n";
          std::cout << "  Packet Delivery Ratio: " << ((i->second.rxPackets * 100) / i->second.txPackets) << "%" << "\n";
      }
     }
  Simulator::Destroy();
}
