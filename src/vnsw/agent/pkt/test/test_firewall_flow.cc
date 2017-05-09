/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#include "base/os.h"
#include "test/test_cmn_util.h"
#include "test_pkt_util.h"
#include "pkt/flow_proto.h"
#include <base/task.h>
#include <base/test/task_test_util.h>

VmInterface *vnet[16];
InetInterface *vhost;
char vhost_addr[32];
char vnet_addr[16][32];

PhysicalInterface *eth;
int hash_id;

void RouterIdDepInit(Agent *agent) {
}

struct PortInfo input1[] = {
    {"vnet1", 1, "1.1.1.1", "00:00:01:01:01:01", 1, 1},
    {"vnet2", 2, "1.1.1.2", "00:00:01:01:01:02", 1, 2},
};

struct PortInfo input2[] = {
    {"vnet3", 3, "1.1.1.3", "00:00:01:01:01:03", 1, 3},
    {"vnet4", 4, "1.1.1.4", "00:00:01:01:01:04", 1, 4},
};

IpamInfo ipam_info[] = {
    {"1.1.1.0", 24, "1.1.1.10"},
};

static void AddTagEntry(const char *tag_name, int id) {

    AddTag(tag_name, id);
}

const VmInterface *GetVmPort(int id) {
    return static_cast<const VmInterface *>(vnet[id]);
}

static bool VmPortSetup(struct PortInfo *input, int count, int aclid) {
    bool ret = true;

    AddIPAM("vn1", ipam_info, 1);
    CreateVmportEnv(input, count,  aclid);
    client->WaitForIdle();

    for (int i = 0; i < count; i++) {
        int id = input[i].intf_id;

        EXPECT_TRUE(VmPortActive(input, i));
        if (VmPortActive(input, i) == false) {
            ret = false;
        }

        if (aclid) {
            EXPECT_TRUE(VmPortPolicyEnable(input, i));
            if (VmPortPolicyEnable(input, i) == false) {
                ret = false;
            }
        }

        vnet[id] = VmInterfaceGet(id);
        if (vnet[id] == NULL) {
            ret = false;
        }

        strcpy(vnet_addr[id], vnet[id]->primary_ip_addr().to_string().c_str());
    }

    eth = EthInterfaceGet("vnet0");
    EXPECT_TRUE(eth != NULL);
    if (eth == NULL) {
        ret = false;
    }

    strcpy(vhost_addr, Agent::GetInstance()->router_id().to_string().c_str());
    return ret;
}

bool Init() {
    boost::system::error_code ec;
    bgp_peer_ = CreateBgpPeer(Ip4Address::from_string("0.0.0.1", ec),
                              "xmpp channel");

    if (VmPortSetup(input1, 2, 0) == false)
        return false;

    if (VmPortSetup(input2, 2, 0) == false)
        return false;

    return true;
}

void Shutdown() {
    DelIPAM("vn1");
    DeleteVmportEnv(input1, 2, false);
    DeleteVmportEnv(input2, 2, true, 1);
    client->WaitForIdle();

    EXPECT_FALSE(VmPortFind(input1, 0));
    EXPECT_FALSE(VmPortFind(input1, 1));
    EXPECT_FALSE(VmPortFind(input2, 0));
    EXPECT_FALSE(VmPortFind(input2, 1));
    DeleteBgpPeer(bgp_peer_);
    client->WaitForIdle();
}

class TagTest : public ::testing::Test {
public:
    virtual void SetUp() {
        agent_ = Agent::GetInstance();
        flow_proto_ = agent_->pkt()->get_flow_proto();
        client->WaitForIdle();
        EXPECT_EQ(0U, flow_proto_->FlowCount());

        const VmInterface *port = GetVmPort(1);
        EXPECT_EQ(port->sg_list().list_.size(), 0U);
        AddTagEntry("tag1", 2);
        AddLink("virtual-machine-interface", "vnet1", "tag", "tag1");
        client->WaitForIdle();
        //EXPECT_EQ(port->tag_list().list_.size(), 1U);
    }

    virtual void TearDown() {
        DelIPAM("vn1");
        client->EnqueueFlowFlush();
        client->WaitForIdle();

        EXPECT_EQ(0U, flow_proto_->FlowCount());
        DelLink("virtual-machine-interface", "vnet1", "tag", "tag1");

        DelNode("tag", "tag1");
        client->WaitForIdle();

        //const VmInterface *port = GetVmPort(1);
        //EXPECT_EQ(port->tag_list().list_.size(), 0U);
    }

    Agent *agent_;
    FlowProto *flow_proto_;
};

// Allow in both forward and reverse directions
TEST_F(TagTest, Flow_Allow_1) {
    TxIpPacket(vnet[1]->id(), vnet_addr[1], vnet_addr[2], 1);
    client->WaitForIdle();
}

int main(int argc, char *argv[]) {
    int ret = 0;

    GETUSERARGS();
    client = TestInit(init_file, ksync_init, true, true, true, 100*1000);
    if (Init()) {
        ret = RUN_ALL_TESTS();
        usleep(100000);
        Shutdown();
    }
    TestShutdown();
    delete client;
    return ret;
}
