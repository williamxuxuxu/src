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

int main (int argc, char *argv[])
{
  // Create nodes
  NodeContainer groundNodes;
  groundNodes.Create(6);  // 6 mobile ground nodes

  NodeContainer uavNode;
  uavNode.Create(1);  // 1 UAV node (server)

  // Install mobility models
  MobilityHelper mobility;
 
  // Set initial positions for ground nodes
  for (uint32_t i = 0; i < 6; ++i) {
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(i * 20.0, 0.0, 0.0)); // Initial positions with a separation of 20 meters
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(groundNodes.Get(i));
  }

  // Set initial position for UAV
  Ptr<ListPositionAllocator> uavPositionAlloc = CreateObject<ListPositionAllocator>();
  uavPositionAlloc->Add(Vector(0.0, 100.0, 10.0)); // Initial position for UAV
  mobility.SetPositionAllocator(uavPositionAlloc);
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  mobility.Install(uavNode);

  // Install AODV routing
  AodvHelper aodv;
  Ipv4ListRoutingHelper list;
  list.Add(aodv, 0);
  InternetStackHelper internet;
  internet.SetRoutingHelper(list);
  internet.Install(groundNodes);
  internet.Install(uavNode);

  // Create the wireless channel
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  //YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  // Set up the Wi-Fi stack
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                               "DataMode", StringValue("OfdmRate6Mbps"));
  //wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

  // Set up MAC layer
  WifiMacHelper mac;
  Ssid ssid = Ssid("network-1");
  mac.SetType("ns3::StaWifiMac",
              "Ssid", SsidValue(ssid),
              "ActiveProbing", BooleanValue(false));

  NetDeviceContainer staDevices = wifi.Install(phy, mac, groundNodes);

  // Set up MAC and PHY for the UAV
  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
  NetDeviceContainer apDevice = wifi.Install(phy, mac, uavNode);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer groundInterfaces = ipv4.Assign (staDevices);
  Ipv4InterfaceContainer uavInterface = ipv4.Assign (apDevice);

  // Create a simple UDP application
  uint16_t port = 9;  // Discard port (RFC 863)
  UdpServerHelper server(port);
  ApplicationContainer serverApp = server.Install(uavNode.Get(0));
 
  serverApp.Start(Seconds(1.0));
  serverApp.Stop(Seconds(9.0));

  UdpClientHelper client(uavInterface.GetAddress(0), port);
  client.SetAttribute("MaxPackets", UintegerValue(10));
  client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  client.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer clientApp = client.Install(groundNodes);
  clientApp.Start(Seconds(2.0));
  clientApp.Stop(Seconds(9.0));

  // Enable NetAnim animation
  AnimationInterface anim("simulation.xml");

  // Run the simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
} 
