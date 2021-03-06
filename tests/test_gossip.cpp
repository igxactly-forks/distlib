// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 2/12/17.


#include <memory>
#include <thread>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "../include/swim/SwimClient.hpp"

#include "../include/swim/GossipFailureDetector.hpp"

#include "tests.h"


using namespace swim;


// Simple implementation of factory method.
namespace swim {
std::unique_ptr<SwimServer> CreateServer(unsigned short port) {
  return std::unique_ptr<SwimServer>(new SwimServer(port));
}
} // namespace swim {


TEST(GossipTests, recordsets) {

  ServerRecordsSet records;

  std::shared_ptr<Server> server = MakeServer("localhost", 8081);
  std::shared_ptr<Server> server2 = MakeServer("localhost", 8088);
  std::shared_ptr<Server> sameServer = MakeServer("localhost", 8081);

  records.insert(MakeRecord(*server));
  records.insert(MakeRecord(*server2));
  ASSERT_EQ(2, records.size());

  // A server with the same hostname/port will be considered equal and thus
  // will NOT be added to the Set.
  records.insert(MakeRecord(*sameServer));
  ASSERT_EQ(2, records.size());
}

TEST(GossipTests, streamOut) {
  ServerRecordsSet records;

  std::shared_ptr<Server> server = MakeServer("localhost", 8081);
  std::shared_ptr<Server> server2 = MakeServer("localhost", 8088);

  records.insert(MakeRecord(*server));
  records.insert(MakeRecord(*server2));

  std::ostringstream s;
  s << records;

  ASSERT_EQ(0, s.str().find("{ ['localhost:8081' at:"));
  ASSERT_LT(s.str().find(", ['localhost:8088' at:"), 100);
}



class GossipFailureDetectorTests : public ::testing::Test {
protected:
  std::unique_ptr<GossipFailureDetector> detector_;
  unsigned short port_;

  void SetUp() override {
    port_ = tests::RandomPort();
    detector_.reset(new GossipFailureDetector(port_, 10, 500, 5));
  }
};


TEST_F(GossipFailureDetectorTests, updatesAlives) {

  SwimClient client(*MakeServer("localhost", detector_->gossip_server().port()), 9000);

  ASSERT_TRUE(tests::WaitAtMostFor([&]() -> bool { return detector_->gossip_server().isRunning(); },
                       std::chrono::milliseconds(2000))) << "Detector didn't start";

  const SwimServer& server = detector_->gossip_server();

  ASSERT_TRUE(server.alive_empty());
  ASSERT_TRUE(client.Ping());
  ASSERT_EQ(1, server.alive_size());
}

TEST_F(GossipFailureDetectorTests, updatesManyAlives) {

  ASSERT_TRUE(tests::WaitAtMostFor([&]() -> bool { return detector_->gossip_server().isRunning(); },
                                   std::chrono::milliseconds(2000))) << "Detector didn't start";
  const SwimServer& server = detector_->gossip_server();

  ASSERT_TRUE(server.alive_empty());
  for (int i = 0; i < 10; ++i) {
    SwimClient client(*MakeServer("localhost", detector_->gossip_server().port()),
                      tests::RandomPort());
    ASSERT_TRUE(client.Ping());
  }

  ASSERT_EQ(10, server.alive_size());
}


TEST_F(GossipFailureDetectorTests, create) {

  Server h1;
  h1.set_hostname("h1");
  h1.set_ip_addr("10.10.1.5");
  h1.set_port(8080);
  detector_->AddNeighbor(h1);
  const SwimServer& server = detector_->gossip_server();

  // Adding twice the same server will have no effect.
  detector_->AddNeighbor(h1);
  ASSERT_EQ(1, server.alive_size());

  // Obviously, a different object makes no difference.
  Server h1_alias;
  h1_alias.set_hostname("h1");
  h1_alias.set_ip_addr("10.10.1.5");
  h1_alias.set_port(8080);

  // Still one server in the set.
  detector_->AddNeighbor(h1_alias);
  ASSERT_EQ(1, server.alive_size());

  // However, a different port is regarded as a different server: note `hostname` is still "h1."
  Server h1_other;
  h1_other.set_hostname("h1");
  h1_other.set_ip_addr("10.10.1.5");
  h1_other.set_port(8090);
  detector_->AddNeighbor(h1_other);

  ASSERT_EQ(2, server.alive_size());
}

TEST_F(GossipFailureDetectorTests, addNeighbors) {
  const SwimServer& server = detector_->gossip_server();
  std::shared_ptr<Server> host1 = MakeServer("host1.example.com", 8087),
      host2 = MakeServer("host2.test.net", 9099);

  ASSERT_TRUE(server.alive_empty());

  detector_->AddNeighbor(*host1);
  detector_->AddNeighbor(*host2);

  ASSERT_EQ(2, server.alive_size());

  std::shared_ptr<Server> server3 = MakeServer("another.example.com", 4456);
  detector_->AddNeighbor(*server3);
  ASSERT_EQ(3, server.alive_size());
}


TEST_F(GossipFailureDetectorTests, prepareReport) {
  for (int i = 0; i < 3; ++i) {
    std::string host = "host_" + std::to_string(i) + ".example.com";
    std::shared_ptr<Server> server = MakeServer(host, 4456 + i);
    detector_->AddNeighbor(*server);
  }

  SwimReport report = detector_->gossip_server().PrepareReport();
  ASSERT_EQ(3, report.alive_size());
  ASSERT_EQ(0, report.suspected_size());

  // We need to cast-away const-ness, so that we can mutate the list of alive
  // and suspected servers.
  // Generally speaking, this is **thread-unsafe** but here we can, as we can be
  // sure there is no other thread accessing these collections.
  auto & swimServer = const_cast<SwimServer&> (detector_->gossip_server());

  swimServer.ReportSuspected(*MakeServer("host_1.example.com", 4457));
  report = detector_->gossip_server().PrepareReport();
  ASSERT_EQ(2, report.alive_size());
  ASSERT_EQ(1, report.suspected_size());
}

